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
        display->drawText(2, 9, network.getDateString().c_str());

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
        int iconChar = getIconChar(AppData.weatherCode);
        display->drawGlyph(4, footerY, iconChar);
        
        char tempStr[16];
        display->setFont(u8g2_font_ncenB08_tr);
        sprintf(tempStr, "%d C", AppData.temperature); // 从 AppData 读
        display->drawText(24, footerY - 1, tempStr);

        // --- 右侧：步数 ---
        // 1. 先设置字体，因为需要计算数字的像素宽度
        display->setFont(u8g2_font_ncenB08_tr);
        char stepStr[16];
        sprintf(stepStr, "%d", AppData.stepCount);
        int stepW = display->getStrWidth(stepStr);
        // 2. 计算坐标
        int iconX = 128 - 16 - 2; 
        int textX = iconX - stepW - 2;

        display->drawIcon(iconX, 48, 16, 16, icon_footprint_16);
        display->drawText(textX, footerY - 1, stepStr);
        display->drawLine(0, 12, 128, 12);
    }

    void onButton(int id) override {
        // ...
    }
private:
    // 将 code 转为 u8g2 字符的简单函数
    int getIconChar(int weatherCode) {
        // 0-3: 晴 -> 69 ('E')
        if (weatherCode <= 3) return 69; 
        // 4-9: 云 -> 64 ('@')
        if (weatherCode <= 9) return 64;
        // 10-18: 雨 -> 67 ('C')
        if (weatherCode <= 18) return 67;
        // 其他 -> 69
        return 69;
    }
};