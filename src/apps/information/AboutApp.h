#pragma once
#include "model/AppBase.h"
#include <Arduino.h>
#include <vector>

class AppController;

class AboutApp : public AppBase {
public:
    AboutApp() {}
    virtual ~AboutApp() {}

    void onRun(AppController* sys) override;
    int  onLoop() override;
    void onExit() override;

private:
    AppController* sys = nullptr;
    
    float currentScrollY = 0.0f; // 当前渲染位置
    int targetScrollY = 0;       // 目标位置
    int totalContentHeight = 0;  // 内容总高度
    const int lineHeight = 14;   // 每行高度
    bool isExiting = false;      // 退出标志位
    std::vector<String> lines;   // 存储所有要显示的行

    void initInfo();             // 初始化信息内容
    void draw();                 // 绘制当前画面
    
    // 按键回调
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();
    
    // 长按连续滚动
    void onKeyHoldUp();
    void onKeyHoldDown();
};