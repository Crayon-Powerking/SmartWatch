#pragma once

// --- 项目元数据 ---
#define PROJECT_NAME "SmartWatch"
#define PROJECT_VERSION "0.1.0"

// --- 硬件引脚定义 (基于 ESP32 Wroom) ---
// OLED 显示屏引脚
#define PIN_OLED_CS   5
#define PIN_OLED_DC   16
#define PIN_OLED_RES  4
// 注意：SDA(23) 和 SCL(18) 是硬件默认引脚，不需要在这里定义

// --- 输入设备引脚 ---
#define PIN_BTN_SELECT  0   // 板载按键 -> 确认/切换
#define PIN_BTN_UP    13  // D13 -> 增加
#define PIN_BTN_DOWN  14  // D14 -> 减少

// --- 用户配置 (User Configuration) ---
// WiFi 设置
// #define WIFI_SSID     "8513"
// #define WIFI_PASS     "ai123456"
#define WIFI_SSID     "User"
#define WIFI_PASS     "Crayon666"
// 天气 API 设置
#define WEATHER_KEY         "SvagmpHNnsfuweKoz"
#define WEATHER_CITY        "hefei"
#define WEATHER_CITY_index  10

// 1. 自动保存间隔 (毫秒)
// 比如：5分钟 = 5 * 60 * 1000 = 300000
#define CONFIG_AUTO_SAVE_INTERVAL  300000 

// 2. 天气更新检查间隔 (毫秒)
// 比如：1小时 = 3600 * 1000
#define CONFIG_WEATHER_INTERVAL    3600000