#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// -- 数据结构 --------------------------------------------------------------------

struct HolidayInfo {
    bool success;     
    char name[32];    
    char date[16];    
    long targetTs;    
    time_t fetchTime; 
};

struct DailyInfo {
    int code;           
    int high;           
    int low;            
};

struct WeatherForecast {
    bool success;
    char cityName[32];
    char cityCode[32];
    DailyInfo days[3];  
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
    
    // 基础连接
    void begin(); 
    void connect(const char* ssid, const char* password);
    void stop();
    void update(); // 维护连接状态
    bool isConnected();
    bool isReady(); // 连上且时间已同步

    // -- 异步天气 API --
    void requestWeatherUpdate(const char* key, const char* city); // 发起请求
    bool isWeatherReady();                                        // 检查结果
    WeatherResult getWeatherResult();                             // 获取结果

    // -- 同步 API (阻塞式) --
    WeatherForecast fetchForecast(const char* key, const char* city);
    HolidayInfo     fetchNextHoliday();

private:
    bool _wifiConnected = false;  
    unsigned long _lastCheckTime = 0; 
    const unsigned long CHECK_INTERVAL = 1000; 

    // NTP 配置
    const long gmtOffset_sec = 8 * 3600;
    const int daylightOffset_sec = 0;
    const char* ntpServer = "ntp1.aliyun.com";

    // -- 后台任务相关 --
    TaskHandle_t _weatherTaskHandle = nullptr; 
    String _targetKey;
    String _targetCity;
    WeatherResult _tempWeather;
    bool _weatherReady = false;

    static void weatherTask(void* parameter); 
    WeatherResult fetchWeather(const char* key, const char* city); // 变为私有
};