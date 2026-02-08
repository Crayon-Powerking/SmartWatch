#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "model/AppData.h"

// -- 类定义 ----------------------------------------------------------------------
class StorageService {
public:
    StorageService();

    void begin();
    void load();
    void save();

    // 模板保存
    template <typename T>
    void saveStruct(const char* key, const T& data) {
        prefs.putBytes(key, &data, sizeof(T));
    }

    // 模板读取
    template <typename T>
    bool loadStruct(const char* key, T& data) {
        if (!prefs.isKey(key)) return false;
        prefs.getBytes(key, &data, sizeof(T));
        return true;
    }

    // 基础类型存取
    long getLong(const char* key, long defaultValue = 0) {
        return prefs.getLong(key, defaultValue);
    }

    void putLong(const char* key, long value) {
        prefs.putLong(key, value);
    }

private:
    Preferences prefs;
};