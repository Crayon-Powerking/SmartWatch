#include "service/StorageService.h"
#include <sys/time.h> 

StorageService::StorageService() {}

void StorageService::begin() {
    prefs.begin("watch", false);
}

void StorageService::load() {
    // 1. 读取业务数据
    AppData.stepCount = prefs.getInt("steps", 0);
    AppData.temperature = prefs.getInt("temp", 0);
    AppData.weatherCode = prefs.getInt("w_code", 99);
    AppData.lastWeatherTime = prefs.getLong("w_time", 0);
    AppData.lastStepDayCode = prefs.getInt("step_day", 0);
    AppData.languageIndex = prefs.getInt("lang", 0);
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

    if (prefs.getInt("lang", 0) != AppData.languageIndex) {
        prefs.putInt("lang", AppData.languageIndex); // 保存语言
    }
}