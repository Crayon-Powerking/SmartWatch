#include "StorageService.h"

StorageService::StorageService() {
}

void StorageService::begin() {
    // 打开一个叫 "watch" 的存储空间
    prefs.begin("watch", false);
}

void StorageService::load() {
    // 读出步数，默认值是 0
    AppData.stepCount = prefs.getInt("steps", 0);
    Serial.printf("[Storage] Loaded Steps: %d\n", AppData.stepCount);
}

void StorageService::save() {
    // 1. 先读一下旧值
    int savedSteps = prefs.getInt("steps", 0);
    
    // 2. 比较：只有当前步数和存的不一样，才真正去写 Flash
    if (savedSteps != AppData.stepCount) {
        prefs.putInt("steps", AppData.stepCount);
        Serial.printf("[Storage] Saved Steps: %d\n", AppData.stepCount);
    } else {
        // Serial.println("[Storage] No change, skip save.");
    }
}