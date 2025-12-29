#include <Arduino.h>
#include "AppConfig.h"
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"

DisplayHAL display;
InputHAL input;
int counter = 0;

void updateScreen(const char* status) {
    display.clear();
    display.drawText(10, 20, status);
    char buf[20];
    sprintf(buf, "Count: %d", counter);
    display.drawText(10, 45, buf);
    display.update();
}

void setup() {
    Serial.begin(115200);
    display.begin();
    input.begin();

    // 使用 Lambda 表达式注册回调 (现代 C++ 写法)
    input.attachClick([](){
        Serial.println("Click!");
        counter++;
        updateScreen("Clicked");
    });

    input.attachLongPress([](){
        Serial.println("Long Press!");
        counter = 0;
        updateScreen("Reset");
    });

    updateScreen("Ready");
}

void loop() {
    input.tick();
    delay(10);
}