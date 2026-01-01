#pragma once
#include "Page.h"
#include "model/MenuTypes.h"

#define ICON_W      32   
#define ICON_H      32   

class PageHorizontalMenu : public Page {
public:
    void setMenu(MenuPage* page) {
        currentData = page;
    }

    void draw(DisplayHAL* display) override {
        if (!currentData || currentData->items.empty()) return;

        int idx = currentData->selectedIndex;
        int total = currentData->items.size();

        // ===============================================
        // 1. 顶部进度条
        // ===============================================
        display->drawLine(0, 2, 128, 2); 
        int barW = 128 / total;
        if (barW < 8) barW = 8; 
        
        int barX = 0;
        if (total > 1) {
            barX = (idx * (128 - barW)) / (total - 1);
        } else {
            barX = (128 - barW) / 2;
        }
        display->drawBox(barX, 0, barW, 5); 

        // ===============================================
        // 2. 绘制中间的主角
        // ===============================================
        int iconCenterY = 16; 
        drawItem(display, idx, 64, iconCenterY);

        // ===============================================
        // 3. 绘制左右尖括号 (使用粗体字符 < 和 >)
        // ===============================================
        // 使用 Helvetica 粗体，出来的效果就是非常干净的尖括号
        display->setFont(u8g2_font_helvB12_tf); 
        
        // 调整垂直居中 (基线位置)
        // 字体高约12，中心在30左右，基线大概在 34
        int arrowBaseline = 35;

        // --- 左箭头 (<) ---
        if (idx > 0) {
            display->drawText(2, arrowBaseline, "<");
        }

        // --- 右箭头 (>) ---
        if (idx < total - 1) {
            // 右对齐计算：128 - 字符宽(约8) - 2
            display->drawText(116, arrowBaseline, ">");
        }
    }

    void onButton(int id) override {}

private:
    MenuPage* currentData = nullptr;

    void drawItem(DisplayHAL* display, int index, int x, int y) {
        MenuItem& item = currentData->items[index];
        int drawX = x - (ICON_W / 2);
        
        if (item.icon) {
            display->drawIcon(drawX, y, ICON_W, ICON_H, item.icon);
        } else {
            display->drawFrame(drawX, y, ICON_W, ICON_H); 
        }

        display->setFont(u8g2_font_ncenB08_tr);
        int strW = display->getStrWidth(item.title.c_str());
        display->drawText(64 - (strW / 2), 62, item.title.c_str());
    }
};