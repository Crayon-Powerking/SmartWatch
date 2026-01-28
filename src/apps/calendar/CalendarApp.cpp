#include "apps/calendar/CalendarApp.h"
#include "controller/AppController.h" 
#include "assets/Lang.h"
#include "assets/AppIcons.h"

extern InputHAL btnSelect;
extern InputHAL btnUp;
extern InputHAL btnDown;

void CalendarApp::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;

    btnSelect.attachClick([this](){
        this->isExiting = true;     // 点击选择键退出
    });
}