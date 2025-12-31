#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "time.h"

struct WeatherResult {
    bool success;     // 是否成功
    int temperature;  // 温度
    int code;         // 天气现象代码
};

class NetworkService {
public:
    NetworkService();

    void begin(const char* ssid, const char* password);
    bool isConnected();
    void tick();

    // 时间相关
    String getTimeString();
    String getDateString();
    int getSecond();
    
    // 天气获取
    WeatherResult fetchWeather(const char* key, const char* city);

private:
    bool _connected = false;
    const char* ntpServer = "ntp1.aliyun.com";
    const long gmtOffset_sec = 8 * 3600; 
    const int daylightOffset_sec = 0;
};