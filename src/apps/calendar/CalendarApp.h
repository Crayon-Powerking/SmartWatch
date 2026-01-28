#pragma once
#include "model/AppBase.h"

class AppController;

class CalendarApp : public AppBase {
public:
    CalendarApp() {}
    virtual ~CalendarApp() {}

    virtual void onRun(AppController* sys) override;
    virtual int onLoop() override;
    virtual void onExit() override;
    
private:
    AppController* sys = nullptr;

    bool isExiting = false;

    void render();        // 负责画图
    void handleInput();   // 负责处理按键
};