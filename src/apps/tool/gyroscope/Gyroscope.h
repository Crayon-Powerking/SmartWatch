#pragma once
#include "model/AppBase.h"
#include "controller/AppController.h"

class Gyroscope : public AppBase {
public:
    Gyroscope(){};
    virtual ~Gyroscope(){};

    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return true; }
private:
    void drawGraphicMode();
    void drawDataMode();

    AppController* sys = nullptr;
    bool isGraphMode = true;
    bool isExiting = false; 
};