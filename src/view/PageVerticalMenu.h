#pragma once
#include "Page.h"
#include "model/MenuTypes.h"
#include <Arduino.h>

// --- 布局配置 ---
#define ITEM_H         16    // 行高
#define VISIBLE_ROWS   4     // 可见行数
#define PADDING_X      6     // 文字左右边距
#define CURSOR_R       3     // 光标圆角
#define CURSOR_X       4     // 光标起始X

// --- 区域限制 ---
#define SCROLL_BAR_W   6     
#define CONTENT_W      (128 - SCROLL_BAR_W - 2) // 内容区总宽 (留2px空隙)
#define MAX_TEXT_W     (CONTENT_W - CURSOR_X - PADDING_X*2) // 文字最大允许宽度

// --- 字体修正 ---
#define FONT_Y_OFFSET  12    

class PageVerticalMenu : public Page {
public:
    void setMenu(MenuPage* page) {
        if (currentData != page) cameraY = 0;
        currentData = page;
    }

    void setVisualIndex(float vIndex) {
        visualIndex = vIndex;
    }

    void draw(DisplayHAL* display) override {
        if (!currentData || currentData->items.empty()) return;
        const auto& items = currentData->items;
        int total = items.size();

        display->setFont(u8g2_font_wqy12_t_gb2312);

        // 1. 计算摄像机 (Camera) 位置
        // ------------------------------------------------
        float targetCameraY = (visualIndex - 1.5f) * ITEM_H;
        float maxCameraY = (total * ITEM_H) - 64;
        if (targetCameraY < 0) targetCameraY = 0;
        if (targetCameraY > maxCameraY) targetCameraY = maxCameraY;
        if (maxCameraY < 0) targetCameraY = 0; // 内容不足一屏
        cameraY = targetCameraY;

        // 2. 绘制光标 (作为背景层)
        // ------------------------------------------------
        // 逻辑：光标单纯只是为了“追随”当前的 visualIndex
        // 它的宽度取决于 visualIndex 此时此刻落在哪里
        
        float cursorY = (visualIndex * ITEM_H) - cameraY;
        
        // 计算光标的目标宽度 (根据 visualIndex 插值)
        int idxA = (int)floor(visualIndex);
        int idxB = idxA + 1;
        if (idxA < 0) idxA = 0; if (idxA >= total) idxA = total - 1;
        if (idxB >= total) idxB = total - 1;

        int wA = getSafeWidth(display, items[idxA].title);
        int wB = getSafeWidth(display, items[idxB].title);

        float progress = visualIndex - idxA;
        float currentW = wA + (wB - wA) * progress;
        
        // 绘制白色光标块
        display->setDrawColor(1);
        int boxH = 14;
        int boxY = (int)(cursorY + (ITEM_H - boxH) / 2);
        display->drawRBox(CURSOR_X, boxY, (int)currentW, boxH, CURSOR_R);

        // 3. 绘制文字 (XOR 叠加层)
        // ------------------------------------------------
        display->setDrawColor(2); // 开启异或模式 (黑底变白，白底变黑)
        display->setFontMode(1);  // 透明背景

        int startRow = (int)(cameraY / ITEM_H);
        int endRow = startRow + VISIBLE_ROWS + 1;

        for (int i = startRow; i < endRow; i++) {
            if (i < 0 || i >= total) continue;

            // 基础坐标
            int drawY = (int)((i * ITEM_H) - cameraY);
            int textY = drawY + FONT_Y_OFFSET;
            int textBaseX = CURSOR_X + PADDING_X;

            // 判断焦点 (0.25 的误差允许光标稍微偏离中心时也保持选中状态)
            bool isFocused = abs(visualIndex - i) < 0.25;
            String text = items[i].title;
            int strW = display->getStrWidth(text.c_str());

            // 设置裁剪窗口：不管怎么滚，文字绝对不能画到右边滚动条去
            display->setClipWindow(0, drawY, CONTENT_W, drawY + ITEM_H);

            if (isFocused) {
                // === 动态模式：选中 ===
                if (strW > MAX_TEXT_W) {
                    // 需要滚动
                    unsigned long now = millis();
                    int speed = 40; 
                    int gap = 30;
                    int cycle = strW + gap;
                    
                    // 核心修正 1: 计算准确的偏移量
                    int offset = (now / speed) % cycle;

                    // 核心修正 2: 修改裁切窗口 (ClipWindow)
                    // 左边界改用 CURSOR_X (光标左边缘)，而不是 0
                    // 这样文字滚到光标左边缘时就会“消失”，不会滚到屏幕最左边
                    display->setClipWindow(CURSOR_X, drawY, CONTENT_W, drawY + ITEM_H);

                    // 绘制第一段文字
                    // textBaseX 本身就是基于 CURSOR_X + PADDING_X 计算的，所以起始位置是对的
                    display->drawText(textBaseX - offset, textY, text.c_str());
                    
                    // 绘制第二段文字 (无缝循环的尾巴)
                    // 当第一段文字往左跑，跑出了一定距离，第二段就要跟上
                    // 只要第二段文字的“头”进入了右边的可见区域 (CONTENT_W)，就把它画出来
                    int secondSegmentX = textBaseX - offset + cycle;
                    if (secondSegmentX < CONTENT_W) {
                         display->drawText(secondSegmentX, textY, text.c_str());
                    }
                    
                    // 记得恢复全屏裁切，以免影响后续绘制
                    display->setMaxClipWindow();

                } else {
                    // 选中但不够长，不需要滚动，直接居中或左对齐显示
                    // 为了统一视觉，最好也加上裁切，防止文字偶然溢出
                    display->setClipWindow(CURSOR_X, drawY, CONTENT_W, drawY + ITEM_H);
                    display->drawText(textBaseX, textY, text.c_str());
                    display->setMaxClipWindow();
                }
            } else {
                // === 静态模式：未选中 ===
                // 如果超长，就截断并加 ...
                if (strW > MAX_TEXT_W) {
                    String truncStr = getTruncatedString(display, text, MAX_TEXT_W);
                    display->drawText(textBaseX, textY, truncStr.c_str());
                } else {
                    display->drawText(textBaseX, textY, text.c_str());
                }
            }

            // 恢复全屏裁剪
            display->setMaxClipWindow();
        }

        // 恢复画笔
        display->setDrawColor(1);
        display->setFontMode(0);

        // 4. 绘制滚动条
        // ------------------------------------------------
        if (total * ITEM_H > 64) {
            int trackX = 128 - (SCROLL_BAR_W / 2) - 1;
            display->drawLine(trackX, 0, trackX, 64);

            int sliderH = (64 * 64) / (total * ITEM_H);
            if (sliderH < 4) sliderH = 4;
            int sliderY = (int)((cameraY / (float)((total * ITEM_H) - 64)) * (64 - sliderH));
            
            display->drawBox(128 - SCROLL_BAR_W + 1, sliderY, SCROLL_BAR_W - 2, sliderH);
        }
    }

    void onButton(int id) override {}

private:
    MenuPage* currentData = nullptr;
    float visualIndex = 0;
    float cameraY = 0;

    // 获取考虑了边界限制的光标宽度
    int getSafeWidth(DisplayHAL* display, String str) {
        int w = display->getStrWidth(str.c_str()) + PADDING_X * 2;
        // 光标最宽不能超过内容区
        if (w > CONTENT_W - CURSOR_X) w = CONTENT_W - CURSOR_X;
        return w;
    }

    // 智能截断字符串
    String getTruncatedString(DisplayHAL* display, String original, int maxWidth) {
        if (display->getStrWidth(original.c_str()) <= maxWidth) return original;
        String dots = "...";
        int dotsW = display->getStrWidth(dots.c_str());
        
        String temp = original;
        // 稍微预判一下，先根据字符数粗略砍一部分，提高效率 (可选)
        // while (display->getStrWidth(temp.c_str()) > maxWidth + 20) {
        //    temp.remove(temp.length() - 1);
        // }
        
        // 精细裁剪
        while (temp.length() > 0) {
            temp.remove(temp.length() - 1);
            if (display->getStrWidth(temp.c_str()) + dotsW <= maxWidth) {
                return temp + dots;
            }
        }
        return dots;
    }
};