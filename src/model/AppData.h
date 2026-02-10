#pragma once

#include <Arduino.h>
#include "model/ConfigModels.h"

// -- 数据模型结构 ----------------------------------------------------------------
struct AppDataModel {
    // 掉电需保存的数据
    SystemConfig systemConfig;        // 系统配置
    UserConfig userConfig;            // 用户配置
    GameRecords gameRecords;          // 游戏记录
    RuntimeCache runtimeCache;        // 运行时缓存
    AlarmSlot alarmSlots[MAX_ALARMS]; // 闹钟数据

    // 实时数据
    int batteryLevel = 0;           // 当前电量百分比 (0-100)
    bool isWifiConnected = false;   // WiFi 连接状态
    bool isBLEConnected = false;    // 蓝牙连接状态
};

// -- 全局变量声明 ----------------------------------------------------------------
extern AppDataModel AppData;