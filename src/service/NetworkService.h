#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "AppConfig.h"

// -- 数据结构 --------------------------------------------------------------------

struct HolidayInfo {
    bool success;     // 数据是否有效
    char name[32];    // 节日名称
    char date[16];    // 节日日期
    long targetTs;    // 目标日期的 0点 时间戳
    time_t fetchTime; // 上次获取数据的时间
};

struct DailyInfo {
    int code;           // 天气代码
    int high;           // 最高温
    int low;            // 最低温
};

struct WeatherForecast {
    bool success;
    char cityName[32];
    char cityCode[32];
    DailyInfo days[3];  // 3天预报
};

struct WeatherResult {
    bool success;
    int temperature;
    int code;
};

// -- 类定义 ----------------------------------------------------------------------
class NetworkService {
public:
    NetworkService();

    void begin(const char* ssid, const char* password);
    void tick();
    bool isConnected();

    // API 请求方法
    WeatherResult   fetchWeather(const char* key, const char* city);
    WeatherForecast fetchForecast(const char* key, const char* city);
    HolidayInfo     fetchNextHoliday();

private:
    bool _connected = false;
    const long gmtOffset_sec = 8 * 3600;
    const int daylightOffset_sec = 0;
    const char* ntpServer = "ntp1.aliyun.com";
};