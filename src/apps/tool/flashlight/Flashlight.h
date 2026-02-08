#pragma once

#include "model/AppBase.h"
#include "controller/AppController.h"

// -- 类定义 ----------------------------------------------------------------------
class Flashlight : public AppBase {
public:
    Flashlight() {}
    virtual ~Flashlight() {}

    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return true; }

private:
    AppController* sys = nullptr;

    // 状态变量
    unsigned long baseActiveTime = 0;
    bool isExiting = false;

    // 按键处理
    void onKeyAny();

    // 渲染方法
    void render();
};