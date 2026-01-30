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

HolidayInfo NetworkService::fetchNextHoliday() {
    HolidayInfo result;
    // 1. 初始化默认值 (防止乱码)
    result.success = false;
    strcpy(result.name, "Loading..."); // 默认名
    strcpy(result.date, "----");
    result.targetTs = 0;
    result.fetchTime = 0;

    if (!isConnected()) return result;

    WiFiClient client;
    HTTPClient http;

    // 2. 请求 API
    // 接口说明: http://timor.tech/api/holiday/next
    // 参数 type=Y 表示返回带年份的日期 (例如 "2026-02-17")
    String url = "http://timor.tech/api/holiday/next?type=Y"; 

    if (http.begin(client, url)) {
        // 设置 User-Agent 伪装成普通设备，防止被某些防火墙拦截
        http.setUserAgent("ESP32-SmartWatch/1.0"); 
        
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            // 3. 解析 JSON
            // 典型返回: {"code":0, "holiday":{"holiday":true, "name":"春节", "date":"2026-02-17", "rest":1}}
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["code"] == 0) {
                const char* name = doc["holiday"]["name"];
                const char* dateStr = doc["holiday"]["date"];
                
                if (name && dateStr) {
                    // 复制数据到结构体
                    strncpy(result.name, name, 31);
                    result.name[31] = '\0'; // 确保封口
                    
                    strncpy(result.date, dateStr, 15);
                    result.date[15] = '\0';
                    
                    // 4. 解析日期字符串 -> 时间戳
                    struct tm tm = {0};
                    int y, m, d;
                    // sscanf 解析 "2026-02-17" 这种格式
                    if (sscanf(dateStr, "%d-%d-%d", &y, &m, &d) == 3) {
                        tm.tm_year = y - 1900; // tm_year 是从1900开始算的
                        tm.tm_mon = m - 1;     // tm_mon 是 0-11
                        tm.tm_mday = d;
                        tm.tm_hour = 0; tm.tm_min = 0; tm.tm_sec = 0;
                        
                        result.targetTs = mktime(&tm); // 转换为 Unix 时间戳
                    }
                    
                    result.success = true;
                    result.fetchTime = time(NULL); // 记录下获取数据的时间
                    
                    Serial.printf("[Network] Next Holiday: %s (%s)\n", result.name, result.date);
                }
            } else {
                Serial.print("[Network] Holiday JSON Error: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.printf("[Network] Holiday HTTP Error: %d\n", httpCode);
        }
        http.end();
    } else {
        Serial.println("[Network] Connect failed");
    }
    
    return result;
}