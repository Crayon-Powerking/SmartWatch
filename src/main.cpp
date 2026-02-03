#include <Arduino.h>
#include "controller/AppController.h"

AppController app;

void setup() {
    Serial.begin(115200);
    app.begin();
}

void loop() {
    app.tick();
}