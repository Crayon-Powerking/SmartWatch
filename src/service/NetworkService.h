#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "AppConfig.h" // 包含心知天气的配置

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

    WeatherResult fetchWeather(const char* key, const char* city);

private:
    bool _connected = false;
    const long  gmtOffset_sec = 8 * 3600; // UTC+8
    const int   daylightOffset_sec = 0;
    const char* ntpServer = "ntp1.aliyun.com";
};