#pragma once
#include "model/MenuTypes.h"

// 前向声明，减少编译依赖
class AppController;

class SettingsBuilder {
public:
    static MenuPage* build(AppController* app);

private:
    // 以后如果设置项太多，可以拆分成更多私有函数
    // static void addWifiSettings(MenuPage* page, AppController* app);
    // static void addDisplaySettings(MenuPage* page, AppController* app);
};