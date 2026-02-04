#pragma once
#include "model/AppBase.h"
#include "controller/AppController.h"

class Flashlight : public AppBase {
public:
    Flashlight(){}
    virtual ~Flashlight() {}

    void onRun(AppController* sys) override;
    int  onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return true; }
private:
    AppController* sys = nullptr;
    unsigned long baseActiveTime = 0;
};
