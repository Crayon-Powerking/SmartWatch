#pragma once
#include <Arduino.h>

// 系统配置结构体
struct SystemConfig {
    int cursorStyle = 0;    // 光标样式
    int colorinverse = 0;   // 颜色反转
    int languageIndex = 0;  // 语言索引
};

// 游戏记录结构体
struct GameRecords {
    long dinoHighScore = 0; // 恐龙高分
};

//  表盘相关结构体
struct RuntimeCache {
    // 步数相关数据
    int stepCount = 0;      // 今日步数
    int lastStepDay = 0;    // 上次记录步数的日期 (格式: YYYYMMDD)

    // 天气简报
    int weatherCode = 99;   // 天气图标代码 (99表示未知)
    int temperature = 0;    // 当前温度
    time_t lastWeatherTime = 0; // 上次更新时间
    char currentCityCode[32] = ""; // 当前选中的城市
};