#pragma once
#include <Arduino.h>
#include "model/ConfigModels.h"

// 定义数据模型结构体
struct AppDataModel {

    // 掉电需保存的数据
    SystemConfig systemConfig;      // 系统配置
    GameRecords gameRecords;        // 游戏记录
    RuntimeCache runtimeCache;      // 运行时缓存

    // 实时数据 (不保存)
    int batteryLevel = 0;           // 当前电量百分比 (0-100)
    bool isWifiConnected = false;   // WiFi 连接状态
};

// 声明一个全局变量，让所有文件都能找到它
extern AppDataModel AppData;