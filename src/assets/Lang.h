#pragma once

enum Language {
    LANG_EN = 0,
    LANG_CN = 1
};

// --- 通用 ---
static const char* STR_BACK[]       = {"< Back",       "< 返回上一级"}; 
static const char* STR_SAVED[]      = {"Saved",        "已保存"};
// --- 主菜单 ---
static const char* STR_SETTINGS[]   = {"Settings",     "系统设置"};
static const char* STR_WEATHER[]    = {"Weather",      "天气预报"};
static const char* STR_TOOLS[]      = {"Tools",        "实用工具"};
static const char* STR_GAME[]       = {"Games",        "娱乐游戏"};
static const char* STR_ABOUT[]      = {"About",        "关于本机"};
static const char* STR_ALARM[]      = {"Alarm",        "闹钟提醒"};
static const char* STR_CALENDAR[]   = {"Calendar",     "日历查看"};

// --- 设置菜单 ---
static const char* STR_LANG_LABEL[] = {"Language",     "多语言设置"}; 
static const char* STR_LANG_VAL[]   = {"English",      "简体中文"}; 
static const char* STR_REBOOT[]     = {"Reboot",       "重启设备"}; // 备用

// --- 游戏名称 (新增) ---
static const char* STR_GAME_DINO[]      = {"Dino Run",         "谷歌小恐龙"};
static const char* STR_GAME_2048[]      = {"2048",             "2048数字版"};
static const char* STR_GAME_TETRIS[]    = {"Tetris",           "俄罗斯方块"};
static const char* STR_GAME_SNAKE[]     = {"Snake",            "贪吃蛇大作战"};
static const char* STR_GAME_INVADER[]   = {"Space Invaders",   "太空侵略者"};
static const char* STR_GAME_FLAPPY[]    = {"Flappy Bird",      "笨鸟先飞"};
static const char* STR_GAME_PONG[]      = {"Pong",             "乒乓球"};
static const char* STR_GAME_PACMAN[]    = {"Pac-Man",          "吃豆人"};
static const char* STR_GAME_MINECRAFT[] = {"Minecraft",        "我的世界(假)"};
static const char* STR_GAME_DOOM[]      = {"Doom",             "毁灭战士"};
static const char* STR_GAME_CYBER[]     = {"Cyberpunk 2077",   "赛博朋克2077"};
static const char* STR_GAME_GENSHIN[]   = {"Genshin Impact",   "原神(启动!)"};

// --- 游戏专用文本 ---
static const char* STR_DINO_GAMEOVER[] = {"GAME OVER", "游戏结束"};
static const char* STR_DINO_CONTINUE[] = {"Continue",  "继续游戏"};
static const char* STR_DINO_QUIT[]     = {"Quit",      "退出游戏"};