#pragma once
#include "model/AppBase.h"
#include "controller/AppController.h"
#include <vector>

class AboutApp : public AppBase {
public:
    AboutApp(AppController* sys);
    virtual ~AboutApp();

    void onRun(AppController* sys) override;
    int  onLoop() override;
    void onExit() override;

private:
    AppController* app;
    
    // --- 滚动核心参数 (保留丝滑滚动的结构) ---
    float currentScrollY = 0.0f; // 当前渲染位置
    int targetScrollY = 0;       // 目标位置
    
    int totalContentHeight = 0;
    const int lineHeight = 14; 
    
    // 内容缓存
    std::vector<String> lines;

    // 内部函数
    void initInfo();
    void draw();
    
    // 按键回调
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();
    
    // 长按连续滚动
    void onKeyHoldUp();
    void onKeyHoldDown();
};