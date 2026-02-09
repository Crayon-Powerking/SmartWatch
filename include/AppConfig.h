#pragma once

// -- 项目元数据 ------------------------------------------------------------------
#define PROJECT_NAME                "SmartWatch"
#define PROJECT_VERSION             "0.1.0"

// -- 硬件引脚定义 (基于 ESP32 Wroom) ---------------------------------------------

// OLED 显示屏 (SPI)
#define PIN_OLED_CS                 5
#define PIN_OLED_DC                 16
#define PIN_OLED_RES                4
// 注意：SDA(23) 和 SCL(18) 是硬件 I2C 默认引脚，无需重定义

// 输入按键
#define PIN_BTN_SELECT              0       // 确认/切换 (板载 BOOT 键)
#define PIN_BTN_UP                  13      // 向上 (D13)
#define PIN_BTN_DOWN                14      // 向下 (D14)

// -- 系统定时配置 (单位: 毫秒) ---------------------------------------------------

// 闹铃显示时长 (1分钟 = 60,000 ms)
#define CONFIG__ALARMING   60000 

// 自动保存间隔 (5分钟 = 300,000 ms)
#define CONFIG_AUTO_SAVE_INTERVAL   300000 

// 天气更新间隔 (1小时 = 3,600,000 ms)
#define CONFIG_WEATHER_INTERVAL     3600000