#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "AppConfig.h"

// --- 节日信息 ---
struct HolidayInfo {
    bool success;           // 数据是否有效
    char name[32];          // 节日名称
    char date[16];          // 节日日期
    long targetTs;          // 目标日期的 0点 时间戳 (用于计算倒计时)
    time_t fetchTime;       // 上次获取数据的时间 (用于判断过期)
};

// --- 单日天气简报 ---
struct DailyInfo {
    int code;       // 天气图标代码 (如 0=晴, 4=多云)
    int high;       // 最高温
    int low;        // 最低温
};

// --- 未来3天预报结果 ---
struct WeatherForecast {
    bool success;
    char cityName[32];  // 获取到的城市名
    char cityCode[32];  // 城市代码
    DailyInfo days[3];  // days[0]=今天, days[1]=明天, days[2]=后天
};

// --- 实况天气结果 ---
struct WeatherResult {
    bool success;
    int temperature;
    int code;
};

class NetworkService {
public:
    NetworkService();
    void begin(const char* ssid, const char* password);                 // 初始化 WiFi 并连接
    void tick();
    bool isConnected();
    // 获取实况 (只给主表盘用)
    WeatherResult fetchWeather(const char* key, const char* city);      // 获取实况
    // 获取3天预报 (给 WeatherApp 用)
    WeatherForecast fetchForecast(const char* key, const char* city);   // 获取3天预报
    HolidayInfo fetchNextHoliday();                                     // 获取下一个节日信息
    
private:
    bool _connected = false;                                            // 当前 WiFi 连接状态
    const long  gmtOffset_sec = 8 * 3600;                               // UTC+8
    const int   daylightOffset_sec = 0;                                 // 无夏令时
    const char* ntpServer = "ntp1.aliyun.com";                          // NTP 服务器
};