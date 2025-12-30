#pragma once
#include "Page.h"
#include "service/NetworkService.h"
#include "model/AppData.h"
#include "assets/AppIcons.h"
#include <stdio.h>

extern NetworkService network;

class PageWatchFace : public Page {
public:
    void draw(DisplayHAL* display) override {
        // ==========================================
        // 1. 顶部 Header
        // ==========================================
        display->setFont(u8g2_font_6x10_tf);
        display->drawText(2, 9, "12-30 Mon"); 

        // --- WiFi 图标 ---
        if (network.isConnected()) {
            // 切换到 OpenIconic Embedded 字库
            display->setFont(u8g2_font_open_iconic_www_1x_t); 
            display->drawGlyph(96, 9, 72); 
        }

        // --- 电池 (代码绘制，无需素材) ---
        display->drawFrame(106, 0, 18, 10); // 外壳
        display->drawBox(124, 3, 2, 4);     // 正极头
        
        // 计算电量条
        int width = (AppData.batteryLevel * 14) / 100;
        if (width > 14) width = 14; 
        if (width < 0) width = 0;
        if (width > 0) display->drawBox(108, 2, width, 6); // 实心条

        // ==========================================
        // 2. 中间时间 (方案 A: Helvetica 24px)
        // ==========================================
        String timeStr = network.getTimeString();
        int sec = network.getSecond();
        char secStr[5];
        sprintf(secStr, ":%02d", sec);

        // 这里的 Y 坐标可能需要根据字体微调，比如 45 或 48
        int TIME_X = 10; 
        int TIME_Y = 42; 
        
        // 1. 设置新字体
        display->setFont(u8g2_font_helvB24_tf); 
        display->drawText(TIME_X, TIME_Y, timeStr.c_str());
        
        // 2. 绘制秒针 (换个小点的配套字体)
        int wMain = display->getStrWidth(timeStr.c_str());
        display->setFont(u8g2_font_helvB14_tf); // 配套的小字体
        display->drawText(TIME_X + wMain + 2, TIME_Y, secStr);

        // ==========================================
        // 3. 底部
        // ==========================================
        int footerY = 64; 

        // --- 左侧：天气 ---
        display->setFont(u8g2_font_open_iconic_weather_2x_t); // 必须切到 Weather 字库
        display->drawGlyph(4, footerY, 69); // 69 = 太阳 ('E')
        
        display->setFont(u8g2_font_ncenB08_tr); // 切回文字
        display->drawText(24, footerY - 1, "26 C"); // y-1 为了微调对齐

        // --- 右侧：步数 ---
        // 1. 先设置字体，因为需要计算数字的像素宽度
        display->setFont(u8g2_font_ncenB08_tr);
        char stepStr[16];
        sprintf(stepStr, "%d", AppData.stepCount);
        int stepW = display->getStrWidth(stepStr);
        // 2. 计算坐标
        // 图标固定在最右侧：屏幕宽(128) - 图标宽(16) - 右边距(2)
        int iconX = 128 - 16 - 2; 
        // 文字坐标：在图标的左边 -> 图标X - 文字宽 - 间距(2)
        int textX = iconX - stepW - 2;

        // 3. 绘制
        // 图标 (假设 icon_footprint_16 是 16x16)
        display->drawIcon(iconX, 48, 16, 16, icon_footprint_16);
        
        // 文字
        display->drawText(textX, footerY - 1, stepStr);

        // 分割线
        display->drawLine(0, 12, 128, 12);
    }

    void onButton(int id) override {
        // ...
    }
};