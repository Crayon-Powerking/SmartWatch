#pragma once
#include "model/MenuTypes.h"

static const char* ICON_RENDER_STAR = (const char*)2;
class AppController;

class SettingsBuilder {
public:
    static MenuPage* build(AppController* sys);
private:
    static void buildLanguagePage(AppController* sys, MenuPage* parent);
    static void buildCursorPage(AppController* sys, MenuPage* parent);
    static void buildSleepPage(AppController* sys, MenuPage* parent);
};