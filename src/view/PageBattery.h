#pragma once
#include "Page.h"
#include <stdio.h>

class PageBattery : public Page {
public:
    void draw(DisplayHAL* display) override {
        display->drawText(10, 20, "--- Battery ---");
        
        char buf[32];
        sprintf(buf, "%d %%", AppData.batteryLevel);
        display->drawText(40, 50, buf);
    }

    void onButton(int id) override {
        // 电量页的逻辑：UP 充电，DOWN 耗电
        if (id == EVENT_KEY_UP) {
            if (AppData.batteryLevel < 100) AppData.batteryLevel++;
        } 
        else if (id == EVENT_KEY_DOWN) {
            if (AppData.batteryLevel > 0) AppData.batteryLevel--;
        }
    }
};