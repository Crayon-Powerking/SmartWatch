#include "service/StorageService.h"
#include <sys/time.h> 

StorageService::StorageService() {}

void StorageService::begin() {
    prefs.begin("watch", false);
}

void StorageService::load() {
    AppData.stepCount = prefs.getInt("steps", 0);             // 步数
    AppData.temperature = prefs.getInt("temp", 0);            // 当前温度
    AppData.weatherCode = prefs.getInt("w_code", 99);         // 天气代码，99表示未知
    AppData.lastWeatherTime = prefs.getLong("w_time", 0);     // 上次获取天气的时间戳
    AppData.lastStepDayCode = prefs.getInt("step_day", 0);    // 上次记录步数的日期代码 (格式: YYYYMMDD)
    AppData.languageIndex = prefs.getInt("lang", 0);          // 当前语言
    AppData.dinoHighScore = prefs.getLong("dino_hs", 0);      // 恐龙高分
    String cityCode = prefs.getString("curr_city", ""); 
    if (cityCode.length() > 0) {
        strncpy(AppData.currentCityCode, cityCode.c_str(), 31);
        AppData.currentCityCode[31] = '\0';
    } else {
        strcpy(AppData.currentCityCode, ""); // 为空
    }
}

void StorageService::save() {
    // 1. 步数
    int currentSteps = AppData.stepCount;
    if (prefs.getInt("steps", -1) != currentSteps) {
        prefs.putInt("steps", currentSteps);
    }
    int currentDay = AppData.lastStepDayCode;
    if (prefs.getInt("step_day", 0) != currentDay) {
        prefs.putInt("step_day", currentDay);
    }

    // 2. 天气温度
    int currentTemp = AppData.temperature;
    if (prefs.getInt("temp", -999) != currentTemp) {
        prefs.putInt("temp", currentTemp);
    }

    // 3. 天气代码
    int currentCode = AppData.weatherCode;
    if (prefs.getInt("w_code", -1) != currentCode) {
        prefs.putInt("w_code", currentCode);
    }

    // 4. 天气更新时间 (这个是不怎么变的，可以存)
    long currentTime = (long)AppData.lastWeatherTime;
    if (prefs.getLong("w_time", 0) != currentTime) {
        prefs.putLong("w_time", currentTime);
    }
    // 5. 语言设置
    if (prefs.getInt("lang", 0) != AppData.languageIndex) {
        prefs.putInt("lang", AppData.languageIndex); // 保存语言
    }
    // 6. 恐龙高分
    long currentDinoHs = AppData.dinoHighScore;
    if (prefs.getLong("dino_hs", 0) != currentDinoHs) {
        prefs.putLong("dino_hs", currentDinoHs);
    }
    // 7. 当前天气城市代码
    String savedCity = prefs.getString("curr_city", "");
    String currentCity = String(AppData.currentCityCode);
    if (savedCity != currentCity) {
        prefs.putString("curr_city", currentCity);
    }
}
