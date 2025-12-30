#include "NetworkService.h"

NetworkService::NetworkService() {
}

void NetworkService::begin(const char* ssid, const char* password) {
    Serial.println("[Network] Connecting to WiFi...");
    Serial.printf("[Network] SSID: %s\n", ssid);

    // 设置 WiFi 模式为基站模式 (Station)
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    // 我们不在这里死等 (while loop)，以免阻塞主程序
    // 让它在后台慢慢连，我们在 tick() 里检查状态
}

bool NetworkService::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkService::tick() {
    // 检查连接状态变化
    if (isConnected() && !_connected) {
        _connected = true;
        Serial.println("[Network] WiFi Connected!");
        Serial.print("[Network] IP: ");
        Serial.println(WiFi.localIP());

        // 连网成功后，立刻开启时间同步
        // 参数: 时区偏移, 夏令时偏移, NTP服务器地址
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        Serial.println("[Network] Syncing Time with Aliyun NTP...");
    }
    else if (!isConnected() && _connected) {
        _connected = false;
        Serial.println("[Network] WiFi Lost!");
    }
}

String NetworkService::getTimeString() {
    struct tm timeinfo;
    // getLocalTime 会尝试从系统内核读取时间
    // 如果还没同步成功，它会返回 false
    if(!getLocalTime(&timeinfo,0)){
        return "--:--"; // 还没获取到时间
    }

    // 格式化输出
    // %02d 意思是: 如果是 5, 显示 "05"
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    return String(timeStr);
}

int NetworkService::getSecond() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo,0)){
        return 0;
    }
    return timeinfo.tm_sec;
}