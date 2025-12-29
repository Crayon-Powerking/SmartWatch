#pragma once

// --- 项目元数据 ---
#define PROJECT_NAME "SmartWatch"
#define PROJECT_VERSION "0.1.0"

// --- 硬件引脚定义 (基于 ESP32 VSPI) ---
// 你的 OLED 屏幕引脚定义在这里，以后改线只改这里
#define PIN_OLED_CS   5
#define PIN_OLED_DC   16
#define PIN_OLED_RES  4
// 注意：SDA(23) 和 SCL(18) 是硬件默认引脚，不需要在这里定义

// --- 输入设备引脚 ---
#define PIN_BTN_BOOT  0  // ESP32 开发板自带的 BOOT 按钮