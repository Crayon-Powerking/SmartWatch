#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "AppConfig.h" // 包含心知天气的配置

// --- 新增：单日天气简报 ---
struct DailyInfo {
    int code;       // 天气图标代码 (如 0=晴, 4=多云)
    int high;       // 最高温
    int low;        // 最低温
};

// --- 新增：未来3天预报结果 ---
struct WeatherForecast {
    bool success;
    String cityName;    // 获取到的城市名
    DailyInfo days[3];  // days[0]=今天, days[1]=明天, days[2]=后天
};

struct WeatherResult {
    bool success;
    int temperature;
    int code;
};

class NetworkService {
public:
    NetworkService();
    void begin(const char* ssid, const char* password);
    void tick();
    bool isConnected();
    // 原有的：获取实况 (只给主表盘用)
    WeatherResult fetchWeather(const char* key, const char* city);

    // 【新增】：获取3天预报 (给 WeatherApp 用)
    // 注意：这将调用心知天气的 /daily.json 接口
    WeatherForecast fetchForecast(const char* key, const char* city);
    
private:
    bool _connected = false;
    const long  gmtOffset_sec = 8 * 3600; // UTC+8
    const int   daylightOffset_sec = 0;
    const char* ntpServer = "ntp1.aliyun.com";
};