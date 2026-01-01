#include <Arduino.h>
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"
#include "controller/AppController.h"

// 1. 实例化硬件 (HAL)
DisplayHAL display;
InputHAL btnSelect(PIN_BTN_SELECT);
InputHAL btnUp(PIN_BTN_UP);
InputHAL btnDown(PIN_BTN_DOWN);

// 2. 实例化大管家
AppController app;

void setup() {
    Serial.begin(115200);
    
    // 初始化硬件
    display.begin();
    btnSelect.begin();
    btnUp.begin();
    btnDown.begin();

    // 把控制权交给 AppController
    app.begin();
}

void loop() {
    // 1. 硬件心跳
    btnSelect.tick();
    btnUp.tick();
    btnDown.tick();
    
    // 2. 业务心跳
    app.tick();
}