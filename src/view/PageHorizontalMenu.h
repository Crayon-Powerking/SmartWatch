#pragma once

#include "Page.h"
#include "model/MenuTypes.h"
#include <Arduino.h>

// -- 布局常量 --------------------------------------------------------------------
#define ICON_W      32      // 图标宽度
#define ICON_H      32      // 图标高度
#define ITEM_SPACE  38.0f   // 图标间距 (中心点距离)
#define BASE_Y      12      // 图标基准Y位置
#define CURVE_STEEP 0.006f  // 抛物线陡峭度参数

// -- 类定义 ----------------------------------------------------------------------
class PageHorizontalMenu : public Page {
public:
    void setMenu(MenuPage* page) {
        currentData = page;
    }

    void setVisualIndex(float vIndex) {
        visualIndex = vIndex;
    }

    // -- 核心渲染 ----------------------------------------------------------------
    void draw(DisplayHAL* display) override {
        if (!currentData || currentData->items.empty()) return;

        int idx = currentData->selectedIndex;
        int total = currentData->items.size();

        // 1. 绘制顶部进度条
        display->drawLine(0, 2, 128, 2);
        float barW = 128.0f / total;
        if (barW < 8) barW = 8;

        float barX = (total > 1) ? (visualIndex * (128.0f - barW)) / (total - 1) : (128.0f - barW) / 2;
        display->drawBox((int)barX, 0, (int)barW, 5);

        // 2. 绘制方向箭头
        display->setFont(u8g2_font_helvB12_tf);
        if (idx > 0) display->drawText(4, 35, "<");
        if (idx < total - 1) display->drawText(115, 35, ">");

        // 3. 绘制图标区域
        display->setClipWindow(24, 0, 104, 64);
        int centerIdx = (int)(visualIndex + 0.5);
        for (int i = centerIdx - 1; i <= centerIdx + 1; i++) {
            if (i < 0 || i >= total) continue;

            float xOffset = (i - visualIndex) * ITEM_SPACE;
            float drawX = 64.0f + xOffset;
            drawDynamicItem(display, i, drawX);
        }
        display->setMaxClipWindow();
    }

private:
    MenuPage* currentData = nullptr;
    float visualIndex = 0;

    // -- 私有辅助绘制 ------------------------------------------------------------
    void drawDynamicItem(DisplayHAL* display, int index, float centerX) {
        MenuItem& item = currentData->items[index];
        float dist = abs(centerX - 64.0f);
        int yOffset = (int)(CURVE_STEEP * dist * dist);
        int finalY = BASE_Y + yOffset;
        int drawXInt = (int)centerX - (ICON_W / 2);

        if (item.icon) {
            display->drawIcon(drawXInt, finalY, ICON_W, ICON_H, item.icon);
        } else {
            display->drawFrame(drawXInt, finalY, ICON_W, ICON_H);
        }

        if (abs(index - visualIndex) < 0.25) {
            display->setFont(u8g2_font_wqy12_t_gb2312);
            int strW = display->getStrWidth(item.title.c_str());
            display->drawText((int)centerX - (strW / 2), 62, item.title.c_str());
        }
    }
};