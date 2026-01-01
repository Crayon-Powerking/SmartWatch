#pragma once
#include "Page.h"
#include "model/MenuTypes.h"
#include <Arduino.h>

#define ICON_W      32
#define ICON_H      32

// 间距保持紧凑
#define ITEM_SPACE  38.0f

// ==========================================
// 【新增参数】控制旋转木马的弧度
// ==========================================
// 基准 Y 坐标 (中心图标顶部距离屏幕上边缘的像素)
// 稍微改小一点(12)，让中心图标看起来更高
#define BASE_Y      12

// 曲线陡峭程度系数 (抛物线开口大小)
// 值越大，两边下沉得越快。建议范围 0.003 ~ 0.01
// 0.006 是一个在 128宽屏幕上看起来比较舒服的弧度
#define CURVE_STEEP 0.006f
// ==========================================


class PageHorizontalMenu : public Page {
public:
    void setMenu(MenuPage* page) {
        currentData = page;
    }

    void setVisualIndex(float vIndex) {
        visualIndex = vIndex;
    }

    void draw(DisplayHAL* display) override {
        if (!currentData || currentData->items.empty()) return;

        int idx = currentData->selectedIndex;
        int total = currentData->items.size();

        // 1. 绘制顶部进度条 (保持不变)
        display->drawLine(0, 2, 128, 2);
        float barW = 128.0f / total;
        if (barW < 8) barW = 8;

        float barX = 0;
        if (total > 1) {
            barX = (visualIndex * (128.0f - barW)) / (total - 1);
        } else {
            barX = (128.0f - barW) / 2;
        }
        display->drawBox((int)barX, 0, (int)barW, 5);

        // 2. 绘制两侧箭头 (保持不变)
        display->setFont(u8g2_font_helvB12_tf);
        int arrowBaseline = 35;
        if (idx > 0) display->drawText(4, arrowBaseline, "<");
        if (idx < total - 1) display->drawText(115, arrowBaseline, ">");

        // 3. 设定裁剪窗口 (保持不变，确保视野范围)
        display->setClipWindow(24, 0, 104, 64);

        int centerIdx = (int)(visualIndex + 0.5);
        // 绘制视野内的图标
        for (int i = centerIdx - 1; i <= centerIdx + 1; i++) {
            if (i < 0 || i >= total) continue;

            // 计算图标中心的绝对 X 坐标
            float xOffset = (i - visualIndex) * ITEM_SPACE;
            float drawX = 64.0f + xOffset;

            // 【重点】这里不再传入固定的 Y 值，而是传入计算好的 drawX
            drawDynamicItem(display, i, drawX);
        }

        display->setMaxClipWindow();
    }

    void onButton(int id) override {}

private:
    MenuPage* currentData = nullptr;
    float visualIndex = 0;

    // 【核心修改】动态计算 Y 轴的绘制函数
    void drawDynamicItem(DisplayHAL* display, int index, float centerX) {
        MenuItem& item = currentData->items[index];

        // 1. 计算距离屏幕中心的像素距离 (取绝对值)
        float dist = abs(centerX - 64.0f);

        // 2. 应用抛物线公式计算下沉偏移量
        // Offset = 系数 * 距离 * 距离
        int yOffset = (int)(CURVE_STEEP * dist * dist);

        // 3. 得出最终绘制的 Y 坐标
        // 距离中心越远，yOffset 越大，图标越靠下
        int finalY = BASE_Y + yOffset;
        int drawXInt = (int)centerX - (ICON_W / 2);

        // 4. 绘制图标 (使用动态计算的 finalY)
        if (item.icon) {
            display->drawIcon(drawXInt, finalY, ICON_W, ICON_H, item.icon);
        } else {
            display->drawFrame(drawXInt, finalY, ICON_W, ICON_H);
        }

        // 5. 绘制文字 (逻辑不变，只显示最中心的)
        // 文字不需要跟着下沉，因为它只在图标到达中心最高点时才显示
        if (abs(index - visualIndex) < 0.25) {
            display->setFont(u8g2_font_ncenB08_tr);
            int strW = display->getStrWidth(item.title.c_str());
            // 文字固定在底部
            display->drawText((int)centerX - (strW / 2), 62, item.title.c_str());
        }
    }
};