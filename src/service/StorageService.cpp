#include "service/StorageService.h"
#include <sys/time.h>

StorageService::StorageService() {}

void StorageService::begin() {
    prefs.begin("watch", false);
}

// -- 数据管理 --------------------------------------------------------------------

void StorageService::load() {
    // 读取系统配置
    if (!loadStruct("sys_cfg", AppData.systemConfig)) {
        AppData.systemConfig = SystemConfig();
    }
    // 用户配置读取
    if (!loadStruct("user_cfg", AppData.userConfig)){
        AppData.userConfig = UserConfig();
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
    saveStruct("user_cfg", AppData.userConfig);
    saveStruct("game_rec", AppData.gameRecords);
    saveStruct("rt_cache", AppData.runtimeCache);
}