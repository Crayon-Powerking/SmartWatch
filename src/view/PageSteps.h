#pragma once
#include "Page.h"
#include <stdio.h> // 用于 sprintf

// [语法解释]: class PageSteps : public Page
// 继承：PageSteps (子) 继承自 Page (父)。
// 意思是：PageSteps 也是一种 Page，它拥有 Page 的所有特征。
class PageSteps : public Page {
public:
    // --- 1. 实现“画画”的家规 ---
    // override 关键字：告诉编译器“我是来覆盖父类那个函数的”，防止你把函数名拼错。
    void draw(DisplayHAL* display) override {
        display->drawText(10, 20, "--- Steps ---");
        
        char buf[32];
        sprintf(buf, "%d", AppData.stepCount);
        display->drawText(40, 50, buf);
    }

    // --- 2. 响应按键 ---
    // 这个页面只关心 UP 和 DOWN，不管 ENTER
    void onButton(int id) override {
        if (id == EVENT_KEY_UP) {
            AppData.stepCount++;
        } 
        else if (id == EVENT_KEY_DOWN) {
            if (AppData.stepCount > 0) AppData.stepCount--;
        }
        // 注意：这里改了数据，但不用在这里调用 draw。
        // draw 会由 Main 循环统一调用，这就是“数据驱动显示”。
    }
};