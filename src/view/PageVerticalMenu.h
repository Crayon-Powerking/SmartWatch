#pragma once
#include "Page.h"
#include "model/MenuTypes.h"
#include <Arduino.h>

// --- 布局参数 (128x64) ---
#define ITEM_H        16    // 行高 (64 / 4 = 16)
#define VISIBLE_ROWS  4     // 一屏显示 4 行
#define SCROLL_BAR_W  6     // 滚动条宽度
#define TEXT_AREA_W   (128 - SCROLL_BAR_W) // 文字区域宽度 (122px)

// 字体垂直修正 (让 12px 字体在 16px 行高中垂直居中)
// U8g2 的基线比较特殊，通常需要向下偏移 12~13px
#define FONT_Y_OFFSET 13    

class PageVerticalMenu : public Page {
public:
    // 传入菜单数据
    void setMenu(MenuPage* page) {
        // 如果换了菜单页面（比如从“设置”进了“关于”），重置视窗
        if (currentData != page) {
            topIndex = 0; 
        }
        currentData = page;
    }

    // 更新光标位置 (由 Controller 传入)
    void setVisualIndex(float vIndex) {
        // 强制转为整数 (列表菜单不需要平滑过渡，那样会晕)
        int targetIndex = (int)round(vIndex);
        
        // --- 核心视窗逻辑 (Gameboy 风格) ---
        // 1. 确保光标不出界
        if (currentData) {
            int maxIdx = currentData->items.size() - 1;
            if (targetIndex < 0) targetIndex = 0;
            if (targetIndex > maxIdx) targetIndex = maxIdx;
        }
        selectedIndex = targetIndex;

        // 2. 推算视窗位置 (topIndex)
        // 如果光标跑到了视窗上面 -> 视窗跟上去
        if (selectedIndex < topIndex) {
            topIndex = selectedIndex;
        }
        // 如果光标跑到了视窗下面 -> 视窗跟下来
        else if (selectedIndex >= topIndex + VISIBLE_ROWS) {
            topIndex = selectedIndex - VISIBLE_ROWS + 1;
        }
    }

    void draw(DisplayHAL* display) override {
        if (!currentData || currentData->items.empty()) return;

        const auto& items = currentData->items;
        int total = items.size();

        // 1. 设置中文字体 (必须！)
        // 确保你的 DisplayHAL 里加载了 u8g2_font_wqy12_t_gb2312
        display->setFont(u8g2_font_wqy12_t_gb2312);

        // ============================================================
        // 区域 A: 绘制左侧列表 (利用 ClipWindow 防止越界)
        // ============================================================
        for (int i = 0; i < VISIBLE_ROWS; i++) {
            // 计算当前行对应的数据索引
            int dataIdx = topIndex + i;
            if (dataIdx >= total) break; // 数据画完了

            // 计算屏幕坐标 Y
            int drawY = i * ITEM_H;
            
            // 是否被选中？
            bool isSelected = (dataIdx == selectedIndex);

            // --- 背景绘制 ---
            if (isSelected) {
                display->setDrawColor(1); // 白笔
                // 画实心白条 (0 ~ 122px)
                display->drawBox(0, drawY, TEXT_AREA_W, ITEM_H);
                display->setDrawColor(0); // 反色 (黑字)
            } else {
                display->setDrawColor(1); // 白字
            }

            // --- 文字绘制 (带跑马灯 & 裁剪) ---
            const char* title = items[dataIdx].title.c_str();
            
            // 1. 开启裁剪窗口 (只允许在当前行的左侧区域画图)
            // x: 0~122, y: drawY ~ drawY+16
            display->setClipWindow(0, drawY, TEXT_AREA_W, drawY + ITEM_H);

            // 2. 计算文字宽度 (使用修复后的 getUTF8Width)
            int strW = display->getStrWidth(title);
            int textX = 4; // 默认左边距

            // 3. 跑马灯逻辑
            // 触发条件：被选中 且 文字宽度超过显示区域
            if (isSelected && strW > (TEXT_AREA_W - 8)) {
                unsigned long now = millis();
                // 速度：40ms 移动 1px
                int speed = 40; 
                // 留 40px 的空白间隔
                int gap = 40; 
                // 周期计算
                int cycle = strW + gap;
                int offset = (now / speed) % cycle;
                
                // 移动起始坐标
                textX = 4 - offset;
                
                // 如果第一段文字滚走了，在后面补画第二段，制造“循环”错觉
                if (textX + strW < TEXT_AREA_W) {
                    display->drawText(textX + cycle, drawY + FONT_Y_OFFSET, title);
                }
            }

            // 4. 真正画字
            display->drawText(textX, drawY + FONT_Y_OFFSET, title);

            // 5. 关闭裁剪 (恢复全屏)
            display->setMaxClipWindow();

            // 6. 恢复画笔颜色 (为下一行做准备)
            display->setDrawColor(1);
        }

        // ============================================================
        // 区域 B: 绘制右侧滚动条
        // ============================================================
        if (total > VISIBLE_ROWS) {
            // 轨道中心线 X
            int trackX = 128 - (SCROLL_BAR_W / 2);
            
            // 1. 画细线轨道
            display->drawLine(trackX, 0, trackX, 64);

            // 2. 计算滑块
            int screenH = 64;
            // 滑块高度 = 屏幕高度 * (视窗行数 / 总行数)
            int sliderH = (screenH * VISIBLE_ROWS) / total;
            if (sliderH < 6) sliderH = 6; // 最小高度限制

            // 可移动的总距离
            int maxMove = screenH - sliderH;
            // 当前进度 (topIndex / 可滚动的最大Index)
            int maxTop = total - VISIBLE_ROWS;
            
            // 滑块 Y 坐标
            int sliderY = (topIndex * maxMove) / maxTop;

            // 3. 画实心滑块
            // x: 128-5 (宽4px)
            display->drawBox(128 - SCROLL_BAR_W + 1, sliderY, SCROLL_BAR_W - 2, sliderH);
        }
    }

    void onButton(int id) override {}

private:
    MenuPage* currentData = nullptr;
    int topIndex = 0;      // 视窗第一行显示的是第几个数据
    int selectedIndex = 0; // 当前选中的绝对索引
};