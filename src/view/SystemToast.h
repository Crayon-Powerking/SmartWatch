#pragma once
#include "hal/DisplayHAL.h"
#include <Arduino.h>
#include <math.h>

class SystemToast {
public:
    // 触发弹窗
    void show(const char* msg, int durationMs = 1500) {
        message = msg;
        startTime = millis();
        totalDuration = durationMs;
        state = STATE_ENTERING;
        isVisible = true;
    }

    // 在每一帧渲染时调用
    void draw(DisplayHAL* display) {
        if (!isVisible) return;

        unsigned long now = millis();
        float progress = 0.0f;
        
        // --- 1. 状态机与动画进度计算 ---
        long elapsed = now - startTime;

        if (state == STATE_ENTERING) {
            // 进场动画 (0.0 -> 1.0)
            progress = (float)elapsed / ANIM_DURATION;
            if (progress >= 1.0f) {
                progress = 1.0f;
                state = STATE_STAYING;
            }
            // 应用 "Q弹" 缓动函数
            progress = easeOutBack(progress);
        } 
        else if (state == STATE_STAYING) {
            progress = 1.0f;
            // 检查是否该退出了
            if (elapsed > (totalDuration - ANIM_DURATION)) {
                state = STATE_EXITING;
                exitStartTime = now;
            }
        } 
        else if (state == STATE_EXITING) {
            // 退场动画 (1.0 -> 0.0)
            long exitElapsed = now - exitStartTime;
            progress = 1.0f - ((float)exitElapsed / ANIM_DURATION);
            
            // 使用简单的平滑退出，不需要回弹
            if (progress <= 0.0f) {
                isVisible = false;
                return;
            }
        }

        // --- 2. 动态布局计算 ---
        display->setFont(u8g2_font_wqy12_t_gb2312);
        
        int textW = display->getStrWidth(message);
        int boxW = textW + 20;  // 文字宽度 + 左右内边距
        int boxH = 20;          // 弹窗高度
        int boxR = 6;           // 圆角半径

        // 计算 Y 轴位置
        // startY = 64 (屏幕外), targetY = 64 - 24 (底部悬浮)
        int targetY = 64 - boxH - 4; // 离底部留 4px 间隙
        int startY = 64; 
        
        // 线性插值混合位置
        int currentY = startY + (targetY - startY) * progress;
        int currentX = (128 - boxW) / 2; // 水平居中

        // --- 3. 绘制 ---
        
        // A. 绘制背景 (白底圆角)
        display->setDrawColor(1);
        display->drawRBox(currentX, currentY, boxW, boxH, boxR);

        // B. 绘制描边 (可选，为了在复杂背景上更清晰，这里画个反色的框)
        // display->setDrawColor(2); // XOR 模式
        // display->drawRFrame(currentX, currentY, boxW, boxH, boxR);

        // C. 绘制文字 (黑字, 居中)
        display->setDrawColor(0); // 反色挖空
        
        // 文字垂直居中修正 (字号12px，高度约12，Box高20，偏移约 (20-12)/2 + ascent)
        // u8g2 的 drawText 坐标是基线，wqy12 的 ascent 约为 9-10
        int textX = currentX + 10; // 左边距
        int textY = currentY + 14; // 经验值微调，使文字垂直居中

        display->drawText(textX, textY, message);

        // D. 恢复绘制颜色
        display->setDrawColor(1);
    }

    bool isShowing() { return isVisible; }

private:
    enum State {
        STATE_ENTERING,
        STATE_STAYING,
        STATE_EXITING
    };

    const char* message = "";
    unsigned long startTime = 0;
    unsigned long exitStartTime = 0;
    int totalDuration = 0;
    bool isVisible = false;
    State state = STATE_ENTERING;

    const int ANIM_DURATION = 300; // 动画时长 300ms

    // --- 缓动算法: EaseOutBack ---
    // x: 0.0 到 1.0
    // c1/c3 是控制回弹力度的系数，1.70158 是标准值，值越大弹得越厉害
    float easeOutBack(float x) {
        const float c1 = 1.70158;
        const float c3 = c1 + 1;
        return 1 + c3 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
    }
};