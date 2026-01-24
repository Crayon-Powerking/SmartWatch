#pragma once
#include "Page.h"
#include "model/AppData.h"
#include "assets/AppIcons.h"
#include <stdio.h>
#include <time.h>

class PageWatchFace : public Page {
public:
    void draw(DisplayHAL* display) override {
        // ==========================================
        // --------------- 顶部状态栏 ---------------
        // ==========================================
        display->setFont(u8g2_font_6x10_tf);
        
        // --- 日期 (左上角) ---
        display->drawText(2, 9, getDateString().c_str());

        // --- WiFi 图标 ---
        if (AppData.isWifiConnected) {
            display->setFont(u8g2_font_open_iconic_www_1x_t); 
            display->drawGlyph(96, 9, 72); 
        }

        // --- 电池 (右上角) ---
        display->setDrawColor(1);
        display->drawFrame(106, 0, 18, 10); // 电池框
        display->drawBox(124, 3, 2, 4);     // 电池头
        
        // 计算电量条宽度 (最大14px)
        int width = (AppData.batteryLevel * 14) / 100;
        if (width > 14) width = 14; 
        if (width > 0) display->drawBox(108, 2, width, 6);

        // 分割线
        display->drawLine(0, 12, 128, 12);

        // ==========================================
        // --------------- 时间显示 ---------------
        // ==========================================
        String timeStr = getTimeString();
        int sec = getSecond();
        char secStr[5];
        sprintf(secStr, ":%02d", sec);

        int TIME_X = 8; 
        int TIME_Y = 44; 
        
        // 主时间
        display->setFont(u8g2_font_helvB24_tf); 
        display->drawText(TIME_X, TIME_Y, timeStr.c_str());
        
        // 秒数 (小字体紧跟其后)
        int wMain = display->getStrWidth(timeStr.c_str());
        display->setFont(u8g2_font_helvB14_tf); 
        display->drawText(TIME_X + wMain + 2, TIME_Y, secStr);

        // ==========================================
        // --------------- 底部状态栏 ---------------
        // ==========================================
        int footerY = 64;               // 屏幕底部坐标
        int iconSize = 16;              // 图标大小 16x16
        int iconY = footerY - iconSize; // 图标 Y = 48
        int textBaseY = footerY - 2;    // 文字基线 Y = 62 (留2px底边距)
        
        // 设置底部通用字体
        display->setFont(u8g2_font_ncenB08_tr); 

        // ------------------------------------------------
        // 左侧锚点：天气 (图标固定 X=0，文字向右生长)
        // ------------------------------------------------
        if (AppData.weatherCode == 99) {
            // 绘制占位符
            display->drawText(2, textBaseY, "--");
            display->drawIcon(0, iconY, iconSize, iconSize, icon_fault);
        } else {
            // 1. 获取并绘制图标
            const uint8_t* weatherIcon = getWeatherIcon(AppData.weatherCode);
            display->drawIcon(0, iconY, iconSize, iconSize, weatherIcon);

            // 2. 绘制温度 (从图标右边 18px 开始)
            char tempStr[16];
            sprintf(tempStr, "%d C", AppData.temperature); 
            display->drawText(18, textBaseY, tempStr);
        }


        // ------------------------------------------------
        // 右侧锚点：步数 (图标固定 X=112，文字向左生长)
        // ------------------------------------------------
        // 1. 绘制脚印图标 (固定在最右侧 128-16 = 112)
        int stepIconX = 128 - iconSize; 
        display->drawIcon(stepIconX, iconY, iconSize, iconSize, icon_footprint);

        // 2. 计算文字位置 (从图标左边倒推)
        char stepStr[16];
        sprintf(stepStr, "%d", AppData.stepCount);
        
        int stepTextW = display->getStrWidth(stepStr);
        // 文字起点 = 图标X(112) - 间距(2) - 文字宽
        int stepTextX = stepIconX - 2 - stepTextW;
        
        display->drawText(stepTextX, textBaseY, stepStr);
    }

private:
    // --- [核心] 天气图标映射逻辑 ---
    // 根据 AppIcons.h 中的资源进行匹配
    const uint8_t* getWeatherIcon(int code) {
        if (code == 1 || code == 3) return icon_weather_sunny_evening;  // 晚上
        if (code == 0 || code == 2) return icon_weather_sunny;          // 白天
        if (code >= 4 && code <= 9) return icon_weather_cloudy;         // 多云/阴
        if (code >= 10 && code <= 19) return icon_weather_rain;         // 雨
        if (code >= 20 && code <= 25) return icon_weather_snow;         // 雪
        if (code >= 26 && code <= 38) return icon_weather_fog;          // 雾/霾/沙
        return icon_fault;                                              // 故障/未知
    }

    // --- 辅助：获取时间字符串 (HH:MM) ---
    String getTimeString() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo, 0)) return "13:14";
        char buf[10];
        sprintf(buf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        return String(buf);
    }

    // --- 辅助：获取日期字符串 (MM-DD Mon) ---
    String getDateString() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo, 0)) return "05-20 Mon";
        char buf[32];
        strftime(buf, 32, "%m-%d %a", &timeinfo);
        return String(buf);
    }

    // --- 辅助：获取秒数 ---
    int getSecond() {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo, 0)) return 0;
        return timeinfo.tm_sec;
    }
};