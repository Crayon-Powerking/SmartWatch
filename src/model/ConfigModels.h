#pragma once

#include <Arduino.h>

// -- 宏定义 ----------------------------------------------------------------------
#define MAX_ALARMS 5

// -- 系统配置 --------------------------------------------------------------------
struct SystemConfig {
    bool colorinverse = false;      // 颜色反转
    int cursorStyle = 0;            // 光标样式 (0:实心, 1:空心)
    int languageIndex = 0;          // 语言索引
    int sleepTimeout = 15;          // 睡眠超时(秒)
};

// -- 用户配置 --------------------------------------------------------------------
struct UserConfig {
    char wifi_ssid  [32] = "User";
    char wifi_pass  [64] = "Crayon666";
    char weather_key[64] = "SvagmpHNnsfuweKoz";
};

// -- 游戏记录 --------------------------------------------------------------------
struct GameRecords {
    long dinoHighScore = 0;         // 恐龙游戏高分记录
};

// -- 运行时缓存 (表盘/传感器) ----------------------------------------------------
struct RuntimeCache {
    // 步数统计
    int stepCount = 0;              // 今日累计步数
    int lastStepDay = 0;            // 上次记录日期 (格式: YYYYMMDD)

    // 环境与天气
    int weatherCode = 99;           // 天气图标代码 (99: 未知)
    int temperature = 0;            // 当前环境温度
    time_t lastWeatherTime = 0;     // 上次天气更新时间戳
    char currentCityCode[32] = "";  // 当前定位城市代码
};

// -- 闹钟数据结构 ----------------------------------------------------------------
struct AlarmSlot {
    uint8_t hour;      // 小时
    uint8_t minute;    // 分钟
    uint8_t weekMask;  // 星期掩码
    bool isOpen;       // 闹钟状态
    bool isUsed;       // 槽位使用状态
};