#include "Flashlight.h"

void Flashlight::onRun(AppController* sys) {
    this->sys = sys;
    this->baseActiveTime = sys->getLastActiveTime();
    sys->display.setAlwaysOn(true); // 设置常亮
};

int Flashlight::onLoop() {
    if (millis() - baseActiveTime < 500) return 0;
        if (sys->getLastActiveTime() > baseActiveTime) {
             return 1;
        }
        return 0;
};

void Flashlight::onExit() {
    sys->display.setAlwaysOn(false); 
};

