#pragma once

#include "Page.h"
#include "model/AppData.h"
#include "assets/AppIcons.h"
#include <stdio.h>
#include <time.h>

// -- 类定义 ----------------------------------------------------------------------
class PageWatchFace : public Page {
public:
    void draw(DisplayHAL* display) override {
        struct tm info;
        bool hasTime = getLocalTime(&info, 10);
        drawStatusBar(display, info, hasTime);
        drawTimeArea(display, info, hasTime);
        drawWeatherStepBar(display);
    }

private:
    // -- 内部辅助绘制 ------------------------------------------------------------
    void drawStatusBar(DisplayHAL* display, struct tm& info, bool hasTime) {
        display->setFont(u8g2_font_6x10_tf);
        char buf[32];
        if (hasTime) {
            strftime(buf, 32, "%m-%d %a", &info);
        } else {
            strcpy(buf, "05-20 ---"); // 没对时显示占位符
        }
        display->drawText(2, 9, buf);

        // 绘制 蓝牙图标
        if (AppData.isBLEConnected) {
            display->setFont(u8g2_font_open_iconic_embedded_1x_t);
            display->drawGlyph(86, 9, 74); 
        }

        // 绘制 WIFI图标
        if (AppData.isWifiConnected) {
            display->setFont(u8g2_font_open_iconic_www_1x_t); 
            display->drawGlyph(96, 9, 72); 
        }

        // 绘制电池条
        display->drawFrame(106, 0, 18, 10);
        display->drawBox(124, 3, 2, 4);
        int width = constrain((AppData.batteryLevel * 14) / 100, 0, 14);
        if (width > 0) display->drawBox(108, 2, width, 6);
        display->drawLine(0, 12, 128, 12);
    }

    // 绘制时间
    void drawTimeArea(DisplayHAL* display, struct tm& info, bool hasTime) {
        char timeStr[10];
        char secStr[5];

        if (hasTime) {
            sprintf(timeStr, "%02d:%02d", info.tm_hour, info.tm_min);
            sprintf(secStr, ":%02d", info.tm_sec);
        } else {
            strcpy(timeStr, "13:14");
            strcpy(secStr, ":00");
        }

        display->setFont(u8g2_font_helvB24_tf); 
        display->drawText(8, 44, timeStr);
        
        int wMain = display->getStrWidth(timeStr);
        display->setFont(u8g2_font_helvB14_tf); 
        display->drawText(8 + wMain + 2, 44, secStr);
    }

    // 绘制底层状态栏
    void drawWeatherStepBar(DisplayHAL* display) {
        display->setFont(u8g2_font_ncenB08_tr); 
        
        // 天气
        const uint8_t* icon = (AppData.runtimeCache.weatherCode == 99) ? icon_fault : getWeatherIcon(AppData.runtimeCache.weatherCode);
        display->drawIcon(0, 48, 16, 16, icon);
        if (AppData.runtimeCache.weatherCode != 99) {
            char tempStr[16];
            sprintf(tempStr, "%d C", AppData.runtimeCache.temperature);
            display->drawText(18, 62, tempStr);
        }

        // 步数
        display->drawIcon(112, 48, 16, 16, icon_footprint);
        char stepStr[16];
        sprintf(stepStr, "%d", AppData.runtimeCache.stepCount);
        display->drawText(110 - display->getStrWidth(stepStr), 62, stepStr);
    }

    // -- 数据获取辅助 ------------------------------------------------------------
    const uint8_t* getWeatherIcon(int code) {
        if (code <= 3) return (code == 1 || code == 3) ? icon_weather_sunny_evening : icon_weather_sunny;
        if (code <= 9) return icon_weather_cloudy;
        if (code <= 19) return icon_weather_rain;
        if (code <= 25) return icon_weather_snow;
        return icon_weather_fog;
    }
};