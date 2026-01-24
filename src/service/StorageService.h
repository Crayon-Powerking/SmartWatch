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

    // 通用保存函数
    template <typename T>
    void saveStruct(const char* key, const T& data) {
        prefs.putBytes(key, &data, sizeof(T));
    }

    // 通用读取函数
    template <typename T>
    bool loadStruct(const char* key, T& data) {
        // 先检查键是否存在
        if (!prefs.isKey(key)) return false;
        // 读出来，填入 data
        prefs.getBytes(key, &data, sizeof(T));
        return true;
    }
    
    // 时间戳的读取函数
    long getLong(const char* key, long defaultValue = 0) {
        return prefs.getLong(key, defaultValue);
    }
    
    // 时间戳的保存函数
    void putLong(const char* key, long value) {
        prefs.putLong(key, value);
    }
private:
    Preferences prefs;   // ESP32 的存储钥匙串
};