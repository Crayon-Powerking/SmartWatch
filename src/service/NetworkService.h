#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include "time.h"

class NetworkService {
public:
    NetworkService();

    // 启动网络服务 (传入 WiFi 账号密码)
    void begin(const char* ssid, const char* password);

    // 检查是否连网
    bool isConnected();

    // 获取当前格式化的时间字符串 (例如 "14:30")
    String getTimeString();

    // 获取当前秒数 (用于控制冒号闪烁)
    int getSecond();

    // 并在内部自动校准时间 (心跳函数)
    void tick();

private:
    // 记录连接状态，防止重复打印日志
    bool _connected = false;
    
    // NTP 服务器地址 (阿里云)
    const char* ntpServer = "ntp1.aliyun.com";
    
    // 时区设置 (GMT+8 北京时间)
    // 8 * 3600 = 28800 秒
    const long gmtOffset_sec = 8 * 3600; 
    const int daylightOffset_sec = 0;
};