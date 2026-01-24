#include "service/NetworkService.h"

NetworkService::NetworkService() {}

void NetworkService::begin(const char* ssid, const char* password) {
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
        // 连上 WiFi 后，立即同步 NTP 时间
        // 时间会自动更新到 ESP32 芯片的 RTC 中，之后调用 getLocalTime 即可
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    }
    else if (!isConnected() && _connected) {
        _connected = false;
    }
}

// 获取实况天气
WeatherResult NetworkService::fetchWeather(const char* key, const char* city) {
    WeatherResult result = {false, 0, 99}; // 初始化默认值

    if (!isConnected()) {
        return result;
    }

    WiFiClientSecure client;
    client.setInsecure(); // 跳过 SSL 验证

    HTTPClient http;
    // 构建 API URL
    String url = "https://api.seniverse.com/v3/weather/now.json?key=" + String(key) + 
                 "&location=" + String(city) + "&language=en&unit=c";
    
    if (http.begin(client, url)) { 
        int httpCode = http.GET();
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
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

// 获取3天预报
WeatherForecast NetworkService::fetchForecast(const char* key, const char* city) {
    WeatherForecast result;
    result.success = false;

    if (!isConnected()) return result;

    WiFiClientSecure client;
    client.setInsecure(); // 跳过 SSL 验证

    HTTPClient http;
    // 构建 3天预报 API URL (daily.json)
    // start=0 表示从今天开始, days=3 表示拿3天
    String url = "https://api.seniverse.com/v3/weather/daily.json?key=" + String(key) + 
                 "&location=" + String(city) + "&language=en&unit=c&start=0&days=3";
    
    if (http.begin(client, url)) { 
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            
            // 稍微加大一点 JSON 缓存，因为 daily 数据比 now 长
            JsonDocument doc; 
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                // 1. 获取城市名 (用于确认)
                const char* namePtr = doc["results"][0]["location"]["name"].as<const char*>();
                if (namePtr) {
                    // 只复制前 31 个字符，留一个位置给结束符 '\0'
                    strncpy(result.cityName, namePtr, sizeof(result.cityName) - 1);
                    result.cityName[sizeof(result.cityName) - 1] = '\0'; // 强制封口，防止溢出
                } else {
                    strcpy(result.cityName, "Unknown");
                }
                // 2. 遍历 daily 数组 (通常只有3个)
                JsonArray daily = doc["results"][0]["daily"];
                for (int i = 0; i < 3 && i < daily.size(); i++) {
                    result.days[i].code = daily[i]["code_day"].as<int>(); // 白天天气代码
                    result.days[i].high = daily[i]["high"].as<int>();     // 最高温
                    result.days[i].low  = daily[i]["low"].as<int>();      // 最低温
                }
                
                result.success = true;
                Serial.printf("[Weather] Forecast Success: %s\n", result.cityName);
            } else {
                Serial.print("[Weather] Forecast JSON Error: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.printf("[Weather] Forecast GET failed: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    return result;
}