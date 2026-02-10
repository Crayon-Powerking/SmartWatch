#include "service/NetworkService.h"

NetworkService::NetworkService() {}

// -- 核心连接逻辑 ----------------------------------------------------------------

void NetworkService::begin() {
    WiFi.mode(WIFI_STA); 
    WiFi.setHostname("ESP32-SmartWatch");
}

void NetworkService::connect(const char* ssid, const char* password) {
    if (isConnected()) {
        WiFi.disconnect();
    }
    _wifiConnected = false;
    WiFi.begin(ssid, password);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void NetworkService::stop() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    _wifiConnected = false;
}

bool NetworkService::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkService::isReady() {
    return isConnected() && (time(nullptr) > 1000000000l);
}

// -- 状态维护 --------------------------------------------------------------------

void NetworkService::update() {
    unsigned long now = millis();
    if (now - _lastCheckTime > CHECK_INTERVAL) {
        _lastCheckTime = now;
        bool currentStatus = isConnected();

        if (currentStatus && !_wifiConnected) {
            _wifiConnected = true;
        } else if (!currentStatus && _wifiConnected) {
            _wifiConnected = false;
        }
    }
}

// -- 异步天气任务 ----------------------------------------------------------------

void NetworkService::requestWeatherUpdate(const char* key, const char* city) {
    if (!isReady()) return;
    if (_weatherTaskHandle != nullptr) return; // 任务已在运行

    _targetKey = String(key);
    _targetCity = String(city);

    xTaskCreate(weatherTask, "Net_Weather", 4096, this, 1, &_weatherTaskHandle);
}

void NetworkService::weatherTask(void* parameter) {
    NetworkService* self = (NetworkService*)parameter;
    
    // 执行耗时请求
    WeatherResult res = self->fetchWeather(self->_targetKey.c_str(), self->_targetCity.c_str());

    if (res.success) {
        self->_tempWeather = res;
        self->_weatherReady = true;
    }

    self->_weatherTaskHandle = nullptr;
    vTaskDelete(NULL);
}

bool NetworkService::isWeatherReady() {
    return _weatherReady;
}

WeatherResult NetworkService::getWeatherResult() {
    _weatherReady = false;
    return _tempWeather;
}

// -- 具体 API 请求实现 ------------------------------------------------------------

WeatherResult NetworkService::fetchWeather(const char* key, const char* city) {
    WeatherResult result = {false, 0, 99};
    if (!isConnected()) return result;

    WiFiClient client;
    HTTPClient http;

    String url = "http://api.seniverse.com/v3/weather/now.json?key=" + String(key) +
                 "&location=" + String(city) + "&language=en&unit=c";

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            JsonDocument doc;
            if (!deserializeJson(doc, payload)) {
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

    if (!isConnected()) {
        return result;
    }
    WiFiClient client;
    HTTPClient http;
    String url = "http://api.seniverse.com/v3/weather/daily.json?key=" + String(key) +
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