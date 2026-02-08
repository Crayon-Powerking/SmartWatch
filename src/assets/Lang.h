#pragma once

enum Language {
    LANG_EN = 0,
    LANG_CN = 1
};

//  -- 通用 ------------------------------------------------------------------
static const char* STR_BACK[]       = {"< Back",       "< 返回"}; 
static const char* STR_SAVED[]      = {"Saved",        "已保存"};
static const char* STR_LOADING[]    = {"Loading...",   "正在加载..."};
static const char* STR_NO_DATA[]    = {"No Data",      "暂无数据"};
static const char* STR_NET_ERR[]    = {"Net Error",    "网络错误"};
static const char* STR_EXIT[]       = {"EXIT",         "退出"};
static const char* STR_DEL[]        = {"DEL",          "删除"};
//  --------------------------------------------------------------------------

// -- 菜单主界面 --------------------------------------------------------------
static const char* STR_SETTINGS[]   = {"Settings",     "系统设置"};
static const char* STR_WEATHER[]    = {"Weather",      "天气预报"};
static const char* STR_TOOLS[]      = {"Tools",        "实用工具"};
static const char* STR_GAME[]       = {"Games",        "娱乐游戏"};
static const char* STR_ABOUT[]      = {"About",        "关于本机"};
static const char* STR_ALARM[]      = {"Alarm",        "闹钟提醒"};
static const char* STR_CALENDAR[]   = {"Calendar",     "日历查看"};
static const char* STR_Exit[]       = {"Exit",         "菜单退出"};
// --------------------------------------------------------------------------

// -- 设置相关 ---------------------------------------------------------------
static const char* STR_LANG_LABEL[] = {"Language",     "语言设置"}; 
static const char* STR_LANG_VAL[]   = {"English",      "简体中文"}; 
static const char* STR_CURSOR[]     = {"Cursor",       "光标设置"}; 
static const char* STR_SOLID[]      = {"Solid",        "实心矩形"};
static const char* STR_HOLLOW[]     = {"Hollow",       "空心矩形"};
static const char* STR_INVERSE[]    = {"Inverse",      "反色设置"};
static const char* STR_SLEEP_VAL[]  = {"Sleep Time",   "息屏时间"};
static const char* STR_SLEEP[2][5]  = {
    {"No Sleep", "15s", "30s", "1min", "5min"},
    {"永不息屏", "15秒", "30秒", "1分钟", "5分钟"}
};
// --------------------------------------------------------------------------

// -- 天气相关 ---------------------------------------------------------------
// 城市管理
static const char* STR_CITY_SELECT[]= {"City Slots",   "城市管理"};
static const char* STR_ADD_CITY[]   = {"+ Add City",   "+ 添加城市"};
static const char* STR_EMPTY_SLOT[] = {"[ Empty ]",    "[ 空槽位 ]"};
static const char* STR_ADD[]        = {"ADD",          "添加"};
static const char* STR_LOCK[]       = {"LOCK",         "锁定"};
static const char* STR_HOLD[]       = {"HOLD",         "长按"};
static const char* STR_TODAY[]      = {"TDY",          "今天"};
static const char* STR_TOMORROW[]   = {"TMR",          "明天"};
static const char* STR_DAY_AFTER[]  = {"DAT",          "后天"};
// 常用城市名称
static const char* STR_Hefei[]      =  {"Hefei",        "合肥"};
static const char* STR_Beijing[]     = {"Beijing",      "北京"};
static const char* STR_Shanghai[]    = {"Shanghai",     "上海"};
static const char* STR_Guangzhou[]   = {"Guangzhou",    "广州"};
static const char* STR_Shenzhen[]    = {"Shenzhen",     "深圳"};
static const char* STR_Chengdu[]     = {"Chengdu",      "成都"};
static const char* STR_Hangzhou[]    = {"Hangzhou",     "杭州"};
static const char* STR_Wuhan[]       = {"Wuhan",        "武汉"};
static const char* STR_Xian[]        = {"Xi'an",        "西安"};
static const char* STR_Nanjing[]     = {"Nanjing",      "南京"};
static const char* STR_Suzhou[]      = {"Suzhou",       "苏州"};
// 天气状态映射 (英文使用缩写以适应屏幕)
// 0-3
static const char* STR_W_SUNNY[]    = {"Sunny",        "晴"}; 
// 4-9
static const char* STR_W_CLOUDY[]   = {"Cloudy",       "多云"};
static const char* STR_W_OVERCAST[] = {"Ovrcst",       "阴"};
// 10-19 (雨 - 使用 L/M/H 代表小/中/大)
static const char* STR_W_SHOWER[]   = {"Shower",       "阵雨"};
static const char* STR_W_THUNDER[]  = {"Storm",        "雷阵雨"}; 
static const char* STR_W_RAIN_S[]   = {"L.Rain",       "小雨"};
static const char* STR_W_RAIN_M[]   = {"M.Rain",       "中雨"};
static const char* STR_W_RAIN_L[]   = {"H.Rain",       "大雨"};
static const char* STR_W_RAIN_XL[]  = {"X.Rain",       "暴雨"};
// 20-25 (雪)
static const char* STR_W_SNOW_S[]   = {"L.Snow",       "小雪"};
static const char* STR_W_SNOW_M[]   = {"M.Snow",       "中雪"};
static const char* STR_W_SNOW_L[]   = {"H.Snow",       "大雪"};
static const char* STR_W_SLEET[]    = {"Sleet",        "雨夹雪"};
// 30-38 (雾/霾)
static const char* STR_W_FOG[]      = {"Fog",          "雾"};
static const char* STR_W_HAZE[]     = {"Haze",         "霾"};
static const char* STR_W_SAND[]     = {"Sand",         "沙尘"};
// 错误处理
static const char* STR_W_UNKNOWN[]  = {"N/A",          "未知"};
// --------------------------------------------------------------------------

// -- 游戏相关 ---------------------------------------------------------------
// 游戏列表
static const char* STR_GAME_DINO[]      = {"Dino Run",         "谷歌小恐龙"};
static const char* STR_GAME_2048[]      = {"2048",             "2048数字版"};
static const char* STR_GAME_TETRIS[]    = {"Tetris",           "俄罗斯方块"};
static const char* STR_GAME_SNAKE[]     = {"Snake",            "贪吃蛇大作战"};
static const char* STR_GAME_INVADER[]   = {"Space Invaders",   "太空侵略者"};
static const char* STR_GAME_FLAPPY[]    = {"Flappy Bird",      "笨鸟先飞"};
static const char* STR_GAME_PONG[]      = {"Pong",             "乒乓球"};
static const char* STR_GAME_PACMAN[]    = {"Pac-Man",          "吃豆人"};
static const char* STR_GAME_MINECRAFT[] = {"Minecraft",        "我的世界"};
static const char* STR_GAME_DOOM[]      = {"Doom",             "毁灭战士"};
static const char* STR_GAME_CYBER[]     = {"Cyberpunk 2077",   "赛博朋克2077"};
static const char* STR_GAME_GENSHIN[]   = {"Genshin Impact",   "原神(启动!)"};
// 游戏内通用
static const char* STR_DINO_GAMEOVER[] = {"GAME OVER", "游戏结束"};
static const char* STR_DINO_CONTINUE[] = {"Continue",  "继续游戏"};
static const char* STR_DINO_QUIT[]     = {"Quit",      "退出游戏"};
// --------------------------------------------------------------------------

// -- 日历相关 ---------------------------------------------------------------
static const char* STR_HOL_NEWYEAR[]   = {"New Year",        "元旦"};
static const char* STR_HOL_SPRING[]    = {"Spring Festival", "春节"};
static const char* STR_HOL_TOMB[]      = {"Tomb Sweeping",   "清明节"};
static const char* STR_HOL_LABOR[]     = {"Labor Day",       "劳动节"};
static const char* STR_HOL_GRAGON[]    = {"Dragon Boat",     "端午节"};
static const char* STR_HOL_MIDAUTUMN[] = {"Mid-Autumn",      "中秋节"};
static const char* STR_HOL_NATIONAL[]  = {"National Day",    "国庆节"};
static const char* STR_UNTIL[]         = {"Until Days",      "距离"};
//今日运势
static const char* STR_CAL_HOLD[]      = {"Hold UP to Check", "按住上键求签"};
static const char* STR_CAL_TODAY[]     = {"Today's Fortune:", "今日运势:"};
static const char* STR_LUCK_0[]        = {"Great Luck",   "大吉大利"};
static const char* STR_LUCK_1[]        = {"Good Luck",    "早晚不出事"};
static const char* STR_LUCK_2[]        = {"Small Luck",   "小吉吉"};
static const char* STR_LUCK_3[]        = {"No Bugs",      "代码无Bug"};
static const char* STR_LUCK_4[]        = {"Don't Deploy", "忌上线"};
// --------------------------------------------------------------------------

// -- 闹钟相关 ---------------------------------------------------------------
static const char* STR_ALARM_TITLE[]   = {"Alarm List",     "闹钟列表"};
static const char* STR_ALARM_ADD[]     = {"Add Alarm",      "添加闹钟"};
static const char* STR_ALARM_NEW[]     = {"NEW",            "新建"};
static const char* STR_ALARM_ON[]      = {"ON",             "开"};
static const char* STR_ALARM_OFF[]     = {"OFF",            "关"};
// --------------------------------------------------------------------------

// -- 工具相关 ---------------------------------------------------------------
static const char* STR_TOOL_FLASH[]  = {"Flashlight",   "手电筒"};
static const char* STR_TOOL_IMU[]    = {"IMU Data",     "陀螺仪"};



