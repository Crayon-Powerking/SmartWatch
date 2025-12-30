#pragma once
#include "Page.h"
#include <stdio.h>

class PageAbout : public Page {
public: 
    void draw(DisplayHAL* display) override {
        display->drawText(10, 20, "--- About ---");
        display->drawText(10, 40, "Version 1.0");
        display->drawText(10, 60, "Base ESP32-wroom");

    }
};