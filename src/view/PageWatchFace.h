#pragma once
#include "Page.h"
#include "model/AppData.h"   // 数据源
#include "assets/AppIcons.h" // 图标
#include <stdio.h>
#include <time.h>            // 新增：直接使用系统时间库

class PageWatchFace : public Page {
public:
    void draw(DisplayHAL* display) override {
        // ==========================================
        // 1. 顶部 Header
        // ==========================================
        display->setFont(u8g2_font_6x10_tf);
        
        // 修改点 A: 使用本地辅助函数获取日期
        display->drawText(2, 9, getDateString().c_str());

        // 修改点 B: 使用 AppData 判断 WiFi 状态
        if (AppData.isWifiConnected) {
            display->setFont(u8g2_font_open_iconic_www_1x_t); 
            display->drawGlyph(96, 9, 72); 
        }

        // --- 电池 (代码绘制) ---
        display->drawFrame(106, 0, 18, 10);
        display->drawBox(124, 3, 2, 4);     
        int width = (AppData.batteryLevel * 14) / 100;
        if (width > 14) width = 14; 
        if (width > 0) display->drawBox(108, 2, width, 6);

        // ==========================================
        // 2. 中间时间
        // ==========================================
        // 修改点 C: 使用本地辅助函数获取时间
        String timeStr = getTimeString();
        int sec = getSecond();
        char secStr[5];
        sprintf(secStr, ":%02d", sec);

        int TIME_X = 10; 
        int TIME_Y = 42; 
        
        display->setFont(u8g2_font_helvB24_tf); 
        display->drawText(TIME_X, TIME_Y, timeStr.c_str());
        
        int wMain = display->getStrWidth(timeStr.c_str());
        display->setFont(u8g2_font_helvB14_tf); 
        display->drawText(TIME_X + wMain + 2, TIME_Y, secStr);

        // ==========================================
        // 3. 底部 (天气 & 步数)
        // ==========================================
        int footerY = 64; 

        // --- 天气 ---
        display->setFont(u8g2_font_open_iconic_weather_2x_t); 
        int iconChar = getIconChar(AppData.weatherCode);
        display->drawGlyph(4, footerY, iconChar); 
        
        display->setFont(u8g2_font_ncenB08_tr); 
        char tempStr[16];
        sprintf(tempStr, "%d C", AppData.temperature); 
        display->drawText(24, footerY - 1, tempStr);

        // --- 步数 ---
        display->setFont(u8g2_font_ncenB08_tr);
        char stepStr[16];
        sprintf(stepStr, "%d", AppData.stepCount);
        int stepW = display->getStrWidth(stepStr);
        int iconX = 128 - 16 - 2; 
        int textX = iconX - stepW - 2;

        display->drawIcon(iconX, 48, 16, 16, icon_footprint_16);
        display->drawText(textX, footerY - 1, stepStr);
        display->drawLine(0, 12, 128, 12);
    }

private:
    // --- 辅助函数：天气图标映射 ---
    int getIconChar(int weatherCode) {
        if (weatherCode <= 3) return 69; // 晴
        if (weatherCode <= 9) return 64; // 云
        if (weatherCode <= 18) return 67; // 雨
        return 69;
    }

    // --- 新增：本地时间获取函数 (完全解耦) ---
    // NetworkService 已经通过 configTime 把时间同步到 ESP32 芯片里了
    // 所以这里直接问芯片要时间，不需要问 NetworkService
    String getTimeString() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo, 0)) return "--:--";
        char buf[10];
        sprintf(buf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        return String(buf);
    }

    String getDateString() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo, 0)) return "01-01 Mon";
        char buf[32];
        strftime(buf, 32, "%m-%d %a", &timeinfo);
        return String(buf);
    }

    int getSecond() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo, 0)) return 0;
        return timeinfo.tm_sec;
    }
};