#include "service/NetworkService.h"

NetworkService::NetworkService() {}

// -- 连接与初始化 ----------------------------------------------------------------

void NetworkService::begin(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}

bool NetworkService::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void NetworkService::tick() {
    if (isConnected() && !_connected) {
        _connected = true;
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    } else if (!isConnected() && _connected) {
        _connected = false;
    }
}

// -- 天气 API 请求 ---------------------------------------------------------------

WeatherResult NetworkService::fetchWeather(const char* key, const char* city) {
    WeatherResult result = {false, 0, 99};

    if (!isConnected()) return result;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = "https://api.seniverse.com/v3/weather/now.json?key=" + String(key) +
                 "&location=" + String(city) + "&language=en&unit=c";

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                result.temperature = doc["results"][0]["now"]["temperature"].as<int>();
                result.code = doc["results"][0]["now"]["code"].as<int>();
                result.success = true;
            }
        }
        http.end();
    }
    return result;
}

WeatherForecast NetworkService::fetchForecast(const char* key, const char* city) {
    WeatherForecast result;
    result.success = false;

    if (!isConnected()) return result;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = "https://api.seniverse.com/v3/weather/daily.json?key=" + String(key) +
                 "&location=" + String(city) + "&language=en&unit=c&start=0&days=3";

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                const char* namePtr = doc["results"][0]["location"]["name"].as<const char*>();
                strncpy(result.cityName, namePtr ? namePtr : "Unknown", sizeof(result.cityName) - 1);
                result.cityName[sizeof(result.cityName) - 1] = '\0';

                JsonArray daily = doc["results"][0]["daily"];
                for (int i = 0; i < 3 && i < (int)daily.size(); i++) {
                    result.days[i].code = daily[i]["code_day"].as<int>();
                    result.days[i].high = daily[i]["high"].as<int>();
                    result.days[i].low = daily[i]["low"].as<int>();
                }
                result.success = true;
            }
        }
        http.end();
    }
    return result;
}

// -- 节日 API 请求 ---------------------------------------------------------------

HolidayInfo NetworkService::fetchNextHoliday() {
    HolidayInfo result;
    result.success = false;
    strcpy(result.name, "Loading...");
    strcpy(result.date, "----");
    result.targetTs = 0;
    result.fetchTime = 0;

    if (!isConnected()) return result;

    WiFiClient client;
    HTTPClient http;
    String url = "http://timor.tech/api/holiday/next?type=Y";

    if (http.begin(client, url)) {
        http.setUserAgent("ESP32-SmartWatch/1.0");
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["code"] == 0) {
                const char* name = doc["holiday"]["name"];
                const char* dateStr = doc["holiday"]["date"];

                if (name && dateStr) {
                    strncpy(result.name, name, 31);
                    result.name[31] = '\0';
                    strncpy(result.date, dateStr, 15);
                    result.date[15] = '\0';

                    struct tm tm = {0};
                    int y, m, d;
                    if (sscanf(dateStr, "%d-%d-%d", &y, &m, &d) == 3) {
                        tm.tm_year = y - 1900;
                        tm.tm_mon = m - 1;
                        tm.tm_mday = d;
                        result.targetTs = mktime(&tm);
                    }
                    result.success = true;
                    result.fetchTime = time(NULL);
                }
            }
        }
        http.end();
    }
    return result;
}