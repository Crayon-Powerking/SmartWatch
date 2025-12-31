#include "service/NetworkService.h"

NetworkService::NetworkService() {}

void NetworkService::begin(const char* ssid, const char* password) {
    Serial.printf("[Network] Connecting to %s ...\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    // 这里不阻塞，连接状态交给 tick() 维护
}

bool NetworkService::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkService::tick() {
    // 检查连接状态变化
    if (isConnected() && !_connected) {
        _connected = true;
        Serial.println("[Network] WiFi Connected!");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    }
    else if (!isConnected() && _connected) {
        _connected = false;
        Serial.println("[Network] Disconnected!");
    }
}

String NetworkService::getTimeString() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo, 0)) return "--:--";
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    return String(timeStr);
}

int NetworkService::getSecond() {
    struct tm timeinfo;
    // 尝试获取本地时间，失败则返回 0
    if(!getLocalTime(&timeinfo, 0)){
        return 0;
    }
    return timeinfo.tm_sec;
}

String NetworkService::getDateString() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo, 0)) return "01-01 Mon";
    char dateStr[32];
    strftime(dateStr, 32, "%m-%d %a", &timeinfo);
    return String(dateStr);
}

WeatherResult NetworkService::fetchWeather(const char* key, const char* city) {
    WeatherResult result = {false, 0, 99}; // 初始化默认值：失败, 0度, 未知代码

    if (!isConnected()) {
        // Serial.println("[Weather] No WiFi.");
        return result;
    }

    // 1. 准备安全客户端 (HTTPS 必须!)
    WiFiClientSecure client;
    client.setInsecure(); // <--- 关键！跳过 SSL 证书验证，否则无法连接心知天气

    HTTPClient http;
    String url = "https://api.seniverse.com/v3/weather/now.json?key=" + String(key) + 
                 "&location=" + String(city) + "&language=en&unit=c";

    Serial.println("[Weather] Requesting API...");
    
    // 2. 发起请求
    // 注意：这里必须传入 client，才能支持 HTTPS
    if (http.begin(client, url)) { 
        int httpCode = http.GET();
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                // Serial.println(payload); // 调试用

                // 3. 解析 JSON (修复：使用 StaticJsonDocument 放在栈上，更安全)
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (!error) {
                    result.temperature = doc["results"][0]["now"]["temperature"].as<int>();
                    result.code = doc["results"][0]["now"]["code"].as<int>();
                    result.success = true;
                    Serial.printf("[Weather] Success: %dC, Code: %d\n", result.temperature, result.code);
                } else {
                    Serial.print("[Weather] JSON Error: ");
                    Serial.println(error.c_str());
                }
            }
        } else {
            Serial.printf("[Weather] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Serial.println("[Weather] Unable to connect to server");
    }

    return result;
}
