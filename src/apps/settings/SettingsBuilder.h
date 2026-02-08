#pragma once

#include "model/MenuTypes.h"

class AppController;

// -- 类定义 ----------------------------------------------------------------------
class SettingsBuilder {
public:
    static MenuPage* build(AppController* sys);

private:
    static void buildLanguagePage(AppController* sys, MenuPage* parent);
    static void buildCursorPage(AppController* sys, MenuPage* parent);
    static void buildSleepPage(AppController* sys, MenuPage* parent);
};