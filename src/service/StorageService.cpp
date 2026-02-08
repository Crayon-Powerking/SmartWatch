#include "service/StorageService.h"
#include <sys/time.h>

StorageService::StorageService() {}

void StorageService::begin() {
    prefs.begin("watch", false);
}

// -- 辅助逻辑 --------------------------------------------------------------------

int getTodayDateCode() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return 0;
    return (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;
}

// -- 数据管理 --------------------------------------------------------------------

void StorageService::load() {
    // 读取系统配置
    if (!loadStruct("sys_cfg", AppData.systemConfig)) {
        AppData.systemConfig = SystemConfig();
    }
    // 读取游戏记录
    if (!loadStruct("game_rec", AppData.gameRecords)) {
        AppData.gameRecords = GameRecords();
    }
    // 表盘数据读取
    if (!loadStruct("rt_cache", AppData.runtimeCache)) {
        AppData.runtimeCache = RuntimeCache();
    }
}

void StorageService::save() {
    saveStruct("sys_cfg", AppData.systemConfig);
    saveStruct("game_rec", AppData.gameRecords);
    saveStruct("rt_cache", AppData.runtimeCache);
}