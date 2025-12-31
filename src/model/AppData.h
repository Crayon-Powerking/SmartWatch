#pragma once
#include <Arduino.h>

// 定义数据模型结构体
struct AppDataModel {
    int stepCount = 0;           // 步数
    int batteryLevel = 0;       // 电量百分比
    int currentScreen = 0;       // 当前屏幕索引
    int temperature = 0;         // 当前温度
    int weatherCode = 99;        // 天气代码，99表示未知
    time_t lastWeatherTime = 0;  // 上次获取天气的时间戳
    // 构造函数: 给个初值
    AppDataModel() {
        stepCount = 0;
        batteryLevel = 80;
        currentScreen = 0;
        temperature = 0;
    }
};

// 声明一个全局变量，让所有文件都能找到它
extern AppDataModel AppData;