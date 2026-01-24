#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "model/AppData.h"

class StorageService {
public:
    StorageService();
    void begin();        // 初始化存储系统
    void load();         // 启动时加载数据到内存
    void save();         // 保存内存数据到存储

private:
    Preferences prefs;   // ESP32 的存储钥匙串
};