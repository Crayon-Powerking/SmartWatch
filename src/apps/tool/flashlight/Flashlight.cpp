#include "apps/tool/flashlight/Flashlight.h"

// -- 按键处理 --------------------------------------------------------------------

void Flashlight::onKeyAny() {
    // 只有在进入 500ms 后按键才有效，防止误触退出
    if (millis() - baseActiveTime > 500) {
        this->isExiting = true;
    }
}

// -- 生命周期 --------------------------------------------------------------------

void Flashlight::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    this->baseActiveTime = millis();

    // 设置屏幕常亮
    sys->display.setAlwaysOn(true);

    // 绑定所有按键：手电筒模式下，按下任意键均视为退出
    auto exitCallback = [this, sys]() {
        if (!sys->processInput()) return; // 检查闹钟
        this->onKeyAny(); 
    };

    sys->btnUp.attachClick(exitCallback);
    sys->btnDown.attachClick(exitCallback);
    sys->btnSelect.attachClick(exitCallback);

    // 禁用长按，防止干扰
    sys->btnSelect.attachLongPress(nullptr);
    sys->btnUp.attachDuringLongPress(nullptr);
    sys->btnDown.attachDuringLongPress(nullptr);
}

int Flashlight::onLoop() {
    if (this->isExiting) return 1;

    render();
    return 0;
}

void Flashlight::onExit() {
    // 恢复屏幕自动休眠设置
    sys->display.setAlwaysOn(false);
}

// -- 绘图渲染 --------------------------------------------------------------------

void Flashlight::render() {
    sys->display.clear();
    
    sys->display.setDrawColor(1);
    sys->display.drawBox(0, 0, 128, 64);
}