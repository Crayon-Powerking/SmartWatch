#pragma once

enum Language {
    LANG_EN = 0,
    LANG_CN = 1
};

// --- 通用 ---
static const char* STR_BACK[]       = {"< Back",       "< 返回"}; 
static const char* STR_SAVED[]      = {"Saved",        "已保存"};
static const char* STR_LOADING[]    = {"Loading...",   "正在加载..."};
static const char* STR_NO_DATA[]    = {"No Data",      "暂无数据"};
static const char* STR_NET_ERR[]    = {"Net Error",    "网络错误"};

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
static const char* STR_REBOOT[]     = {"Reboot",       "重启设备"}; 

// --- 天气 App 专用 (新增) ---
static const char* STR_CITY_SELECT[]= {"City Slots",   "城市管理"};
static const char* STR_ADD_CITY[]   = {"+ Add City",   "+ 添加城市"};
static const char* STR_EMPTY_SLOT[] = {"[ Empty ]",    "[ 空槽位 ]"};
static const char* STR_HOLD_DEL[]   = {"Hold to Del",  "长按删除"};
static const char* STR_TODAY[]      = {"TDY",        "今天"};
static const char* STR_TOMORROW[]   = {"TMR",          "明天"};
static const char* STR_DAY_AFTER[]  = {"DAT",          "后天"};

// --- 天气状态映射 (英文使用缩写以适应屏幕) ---
// 0-3
static const char* STR_W_SUNNY[]    = {"Sunny",        "晴"}; // 够短
// 4-9
static const char* STR_W_CLOUDY[]   = {"Cloudy",       "多云"};
static const char* STR_W_OVERCAST[] = {"Ovrcst",       "阴"}; // Overcast -> Ovrcst
// 10-19 (雨 - 使用 L/M/H 代表小/中/大)
static const char* STR_W_SHOWER[]   = {"Shower",       "阵雨"};
static const char* STR_W_THUNDER[]  = {"Storm",        "雷阵雨"}; // Thunderstorm -> Storm
static const char* STR_W_RAIN_S[]   = {"L.Rain",       "小雨"};   // Light Rain -> L.Rain
static const char* STR_W_RAIN_M[]   = {"M.Rain",       "中雨"};   // Moderate -> M.Rain
static const char* STR_W_RAIN_L[]   = {"H.Rain",       "大雨"};   // Heavy -> H.Rain
static const char* STR_W_RAIN_XL[]  = {"X.Rain",       "暴雨"};   // Storm -> X.Rain
// 20-25 (雪)
static const char* STR_W_SNOW_S[]   = {"L.Snow",       "小雪"};
static const char* STR_W_SNOW_M[]   = {"M.Snow",       "中雪"};
static const char* STR_W_SNOW_L[]   = {"H.Snow",       "大雪"};
static const char* STR_W_SLEET[]    = {"Sleet",        "雨夹雪"};
// 30-38 (雾/霾)
static const char* STR_W_FOG[]      = {"Fog",          "雾"};
static const char* STR_W_HAZE[]     = {"Haze",         "霾"};
static const char* STR_W_SAND[]     = {"Sand",         "沙尘"};

static const char* STR_W_UNKNOWN[]  = {"N/A",          "未知"};

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