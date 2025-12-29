#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "model/AppData.h"

class StorageService {
public:
    StorageService();

    void begin();
    
    // 开机时：从硬盘把步数读出来
    void load();
    
    // 关机前或定时：把步数存进硬盘
    void save();

private:
    Preferences prefs; // ESP32 的存储钥匙串
};