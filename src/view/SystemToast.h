#pragma once
#include "hal/DisplayHAL.h"
#include <Arduino.h>

class SystemToast {
public:
    // 触发弹窗
    void show(const char* msg, int durationMs = 1500) {
        message = msg;
        startTime = millis();
        duration = durationMs;
        isVisible = true;
    }

    // 在每一帧渲染时调用
    void draw(DisplayHAL* display) {
        // 1. 检查是否过期
        if (!isVisible) return;
        if (millis() - startTime > duration) {
            isVisible = false;
            return;
        }

        // 2. 定义样式 (底部条状)
        // 屏幕 128x64。底部留 16px 给它 (y=48~64)
        int h = 16;
        int y = 64 - h; // y = 48
        int w = 52;
        int x = 38;

        // 3. 绘制背景 (白底)
        display->setDrawColor(1);
        display->drawBox(x, y, w, h);

        // 4. 绘制文字 (黑字, 居中)
        display->setDrawColor(0); // 反色
        
        // 必须设置中文字体
        display->setFont(u8g2_font_wqy12_t_gb2312); 

        int strW = display->getStrWidth(message);
        int textX = (128 - strW) / 2;
        int textY = y + 13; // 垂直微调

        display->drawText(textX, textY, message);

        // 5. 恢复颜色
        display->setDrawColor(1);
    }

    bool isShowing() { return isVisible; }

private:
    const char* message = "";
    unsigned long startTime = 0;
    int duration = 0;
    bool isVisible = false;
};