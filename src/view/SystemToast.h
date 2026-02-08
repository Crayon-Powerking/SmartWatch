#pragma once

#include "hal/DisplayHAL.h"
#include <Arduino.h>
#include <math.h>

// -- 类定义 ----------------------------------------------------------------------
class SystemToast {
public:
    void show(const char* msg, int durationMs = 1500) {
        message = msg;
        startTime = millis();
        totalDuration = durationMs;
        state = STATE_ENTERING;
        isVisible = true;
    }

    void hide() {
        isVisible = false;
        state = STATE_EXITING; // 或者直接重置状态
    }
    
    void draw(DisplayHAL* display) {
        if (!isVisible) return;

        // 1. 动画状态机逻辑
        updateAnimation();

        // 2. 布局计算
        display->setFont(u8g2_font_wqy12_t_gb2312);
        int textW = display->getStrWidth(message);
        int boxW = textW + 20;
        int currentY = 64 + (int)((-24) * currentProgress); // 从64弹到40
        int currentX = (128 - boxW) / 2;

        // 3. 绘制
        display->setDrawColor(1);
        display->drawRBox(currentX, currentY, boxW, 20, 6);
        display->setDrawColor(0);
        display->drawText(currentX + 10, currentY + 14, message);
        display->setDrawColor(1);
    }

    bool isShowing() { return isVisible; }

private:
    enum State { STATE_ENTERING, STATE_STAYING, STATE_EXITING };

    const char* message = "";
    unsigned long startTime = 0;
    unsigned long exitStartTime = 0;
    int totalDuration = 0;
    bool isVisible = false;
    float currentProgress = 0;
    State state = STATE_ENTERING;

    // -- 内部逻辑 ----------------------------------------------------------------
    
    float easeOutBack(float x) {
        const float c1 = 1.70158;
        const float c3 = c1 + 1;
        return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
    }

    void updateAnimation() {
        unsigned long now = millis();
        long elapsed = now - startTime;

        if (state == STATE_ENTERING) {
            float p = (float)elapsed / 300;
            currentProgress = (p >= 1.0f) ? (state = STATE_STAYING, 1.0f) : easeOutBack(p);
        } 
        else if (state == STATE_STAYING) {
            currentProgress = 1.0f;
            if (elapsed > (totalDuration - 300)) {
                state = STATE_EXITING;
                exitStartTime = now;
            }
        } 
        else {
            float p = 1.0f - (float)(now - exitStartTime) / 300;
            if (p <= 0) isVisible = false;
            currentProgress = p;
        }
    }

};