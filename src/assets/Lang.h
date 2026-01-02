#pragma once

// 语言索引
enum Language {
    LANG_EN = 0,
    LANG_CN = 1
};

// ---------------------------------------------------------
// 字典定义
// [0] = 英文, [1] = 中文
// ---------------------------------------------------------

// --- 1. 主菜单 (一级) ---
static const char* STR_SETTINGS[]   = {"Settings",     "系统设置"};
static const char* STR_WEATHER[]    = {"Weather",      "天气预报"};
static const char* STR_TOOLS[]      = {"Tools",        "实用工具"};
static const char* STR_GAME[]       = {"Games",        "娱乐游戏"};
static const char* STR_ABOUT[]      = {"About",        "关于本机"};
static const char* STR_ALARM[]      = {"Alarm",        "闹钟提醒"};
static const char* STR_CALENDAR[]   = {"Calendar",     "日历查看"};

// --- 2. 设置菜单 (二级) ---
// 注意：语言切换项本身也需要双语
static const char* STR_LANG_LABEL[] = {"Language",     "语言设置"}; 
static const char* STR_LANG_VAL[]   = {"English",      "简体中文"}; 

static const char* STR_BRIGHTNESS[] = {"Brightness",   "屏幕亮度"};
static const char* STR_SOUND[]      = {"Sound",        "声音设置"};
static const char* STR_REBOOT[]     = {"Reboot",       "重启设备"};
static const char* STR_FACTORY[]    = {"Reset",        "恢复出厂"};

// 【新增】保存提示
static const char* STR_SAVED[]      = {"Saved!",       "已保存"};