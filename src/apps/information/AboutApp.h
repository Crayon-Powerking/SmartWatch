#pragma once

#include "model/AppBase.h"
#include <Arduino.h>
#include <vector>

class AppController;

// -- 类定义 ----------------------------------------------------------------------
class AboutApp : public AppBase {
public:
    AboutApp() {}
    virtual ~AboutApp() {}

    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return false; }

private:
    AppController* sys = nullptr;

    // 状态变量
    float currentScrollY = 0.0f; // 当前渲染位置
    float targetScrollY = 0.0f;  // 目标位置
    int totalContentHeight = 0;  // 内容总高度
    const int lineHeight = 14;   // 每行高度
    bool isExiting = false;      // 退出标志位
    
    std::vector<String> lines;   // 存储所有要显示的行

    // 内部逻辑
    void initInfo();

    // 按键处理
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();
    void onKeyHoldUp();
    void onKeyHoldDown();

    // 渲染方法
    void draw();
};