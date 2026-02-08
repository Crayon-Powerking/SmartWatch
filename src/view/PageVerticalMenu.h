#pragma once

#include "Page.h"
#include "model/MenuTypes.h"
#include "model/AppData.h"
#include <Arduino.h>

// -- 布局配置 --------------------------------------------------------------------
#define VISIBLE_ROWS   4
#define PADDING_X      6
#define CURSOR_R       3
#define CURSOR_X       4
#define SCROLL_BAR_W   6
#define CONTENT_W      (128 - SCROLL_BAR_W - 2)
#define MAX_TEXT_W     (CONTENT_W - CURSOR_X - PADDING_X * 2)
#define FONT_Y_OFFSET  12

// -- 类定义 ----------------------------------------------------------------------
class PageVerticalMenu : public Page {
public:
    void setMenu(MenuPage* page) {
        if (currentData != page) cameraY = 0;
        currentData = page;
    }

    void setVisualIndex(float vIndex) {
        visualIndex = vIndex;
    }

    // -- 核心渲染 ----------------------------------------------------------------
    void draw(DisplayHAL* display) override {
        if (!currentData || currentData->items.empty()) return;
        const auto& items = currentData->items;
        int total = items.size();

        // 1. 摄像机逻辑
        float targetCameraY = (visualIndex - 1.5f) * LINE_Y;
        float maxCameraY = (total * LINE_Y) - 64;
        cameraY = constrain(targetCameraY, 0, (maxCameraY < 0 ? 0 : maxCameraY));

        // 2. 绘制光标
        drawCursor(display, items);

        // 3. 绘制文字列表
        display->setDrawColor(2);
        display->setFontMode(1);
        display->setInvert(AppData.systemConfig.colorinverse);

        int startRow = (int)(cameraY / LINE_Y);
        for (int i = startRow; i < startRow + VISIBLE_ROWS + 1; i++) {
            if (i < 0 || i >= total) continue;
            drawItemText(display, items[i], i);
        }

        display->setDrawColor(1);
        display->setFontMode(0);

        // 4. 绘制滚动条
        drawScrollBar(display, total);
    }

private:
    MenuPage* currentData = nullptr;
    float visualIndex = 0;
    float cameraY = 0;

    // -- 内部辅助逻辑 ------------------------------------------------------------
    void drawCursor(DisplayHAL* display, const std::vector<MenuItem>& items) {
        float cursorY = (visualIndex * LINE_Y) - cameraY;
        int idxA = constrain((int)floor(visualIndex), 0, (int)items.size() - 1);
        int idxB = constrain(idxA + 1, 0, (int)items.size() - 1);

        int wA = getSafeWidth(display, items[idxA].title);
        int wB = getSafeWidth(display, items[idxB].title);
        float currentW = wA + (wB - wA) * (visualIndex - idxA);

        display->setDrawColor(1);
        int boxY = (int)(cursorY + (LINE_Y - 14) / 2);
        if (AppData.systemConfig.cursorStyle == 0) {
            display->drawRBox(CURSOR_X, boxY, (int)currentW, 14, CURSOR_R);
        } else {
            display->drawRFrame(CURSOR_X, boxY, (int)currentW, 14, CURSOR_R);
        }
    }

    void drawItemText(DisplayHAL* display, const MenuItem& item, int index) {
        int drawY = (int)((index * LINE_Y) - cameraY);
        int textBaseX = CURSOR_X + PADDING_X;
        int strW = display->getStrWidth(item.title.c_str());
        bool isFocused = abs(visualIndex - index) < 0.25;

        display->setClipWindow(CURSOR_X, drawY, CONTENT_W, drawY + LINE_Y);
        if (isFocused && strW > MAX_TEXT_W) {
            int cycle = strW + 30;
            int offset = (millis() / 40) % cycle;
            display->drawText(textBaseX - offset, drawY + FONT_Y_OFFSET, item.title.c_str());
            if (textBaseX - offset + cycle < CONTENT_W) {
                display->drawText(textBaseX - offset + cycle, drawY + FONT_Y_OFFSET, item.title.c_str());
            }
        } else {
            String text = (strW > MAX_TEXT_W) ? getTruncatedString(display, item.title, MAX_TEXT_W) : item.title;
            display->drawText(textBaseX, drawY + FONT_Y_OFFSET, text.c_str());
        }
        display->setMaxClipWindow();
    }

    void drawScrollBar(DisplayHAL* display, int total) {
        if (total * LINE_Y <= 64) return;
        int trackX = 128 - (SCROLL_BAR_W / 2) - 1;
        display->drawLine(trackX, 0, trackX, 64);
        int sliderH = max(4, (64 * 64) / (total * LINE_Y));
        int sliderY = (int)((cameraY / ((total * LINE_Y) - 64)) * (64 - sliderH));
        display->drawBox(128 - SCROLL_BAR_W + 1, sliderY, SCROLL_BAR_W - 2, sliderH);
    }

    int getSafeWidth(DisplayHAL* display, String str) {
        return min((int)(display->getStrWidth(str.c_str()) + PADDING_X * 2), (int)(CONTENT_W - CURSOR_X));
    }

    String getTruncatedString(DisplayHAL* display, String original, int maxWidth) {
        String temp = original;
        int dotsW = display->getStrWidth("...");
        while (temp.length() > 0 && (display->getStrWidth(temp.c_str()) + dotsW > maxWidth)) {
            temp.remove(temp.length() - 1);
        }
        return temp + "...";
    }
};