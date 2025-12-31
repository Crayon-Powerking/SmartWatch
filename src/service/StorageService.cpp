#include "StorageService.h"
#include <sys/time.h>

StorageService::StorageService() {
}

void StorageService::begin() {
    // 打开一个叫 "watch" 的存储空间
    prefs.begin("watch", false);
}

void StorageService::load() {
    // 1. 读取步数等业务数据
    AppData.stepCount = prefs.getInt("steps", 0);
    AppData.temperature = prefs.getInt("temp", 0);
    AppData.weatherCode = prefs.getInt("w_code", 99);
    AppData.lastWeatherTime = prefs.getLong("w_time", 0);

    // 2. --- 核心修改：读取并恢复时间 ---
    // 读取存入的时间戳 (Unix Timestamp)
    time_t savedTime = prefs.getULong("sys_time", 0);
    
    // 如果读出来的时间有效 (比如大于 2020年)，就设置进系统
    if (savedTime > 1577836800) { 
        struct timeval tv;
        tv.tv_sec = savedTime;
        tv.tv_usec = 0;
        
        // 这一步是关键：直接修改 ESP32 的RTC系统时间！
        // 之后调用 getLocalTime 就会立即返回这个时间
        settimeofday(&tv, NULL); 
        
        Serial.printf("[Storage] System time restored to: %ld\n", savedTime);
    }

    Serial.printf("[Storage] Loaded Steps:%d, Temp:%d\n", AppData.stepCount, AppData.temperature);
}

#include "service/StorageService.h"
#include <sys/time.h> 

void StorageService::save() {
    bool isDataChanged = false; // 用于记录是否有数据发生了改变

    // ==================================================
    // 1. 保存步数 (Steps)
    // ==================================================
    int currentSteps = AppData.stepCount;
    // 读取旧值，如果不存在则默认为 -1 (确保第一次肯定能存进去，或者用 0 也可以)
    int savedSteps = prefs.getInt("steps", -1); 
    
    if (savedSteps != currentSteps) {
        prefs.putInt("steps", currentSteps);
        isDataChanged = true;
        // Serial.printf("[Storage] Steps updated: %d -> %d\n", savedSteps, currentSteps);
    }

    // ==================================================
    // 2. 保存天气 - 温度 (Temperature)
    // ==================================================
    int currentTemp = AppData.temperature;
    int savedTemp = prefs.getInt("temp", -999); // -999 是个不可能的温度，防止默认值冲突
    
    if (savedTemp != currentTemp) {
        prefs.putInt("temp", currentTemp);
        isDataChanged = true;
    }

    // ==================================================
    // 3. 保存天气 - 现象代码 (Weather Code)
    // ==================================================
    int currentCode = AppData.weatherCode;
    int savedCode = prefs.getInt("w_code", -1); // -1 代表无效代码
    
    if (savedCode != currentCode) {
        prefs.putInt("w_code", currentCode);
        isDataChanged = true;
    }

    // ==================================================
    // 4. 保存天气 - 更新时间 (Weather Time)
    // ==================================================
    // time_t 本质通常是 long 或 long long，这里用 Long 存储
    long currentTime = (long)AppData.lastWeatherTime;
    long savedTime = prefs.getLong("w_time", 0);
    
    if (savedTime != currentTime) {
        prefs.putLong("w_time", currentTime);
        isDataChanged = true;
    }

    // ==================================================
    // 5. 保存系统时间 (System Time)
    // ==================================================
    // 逻辑：时间是一直在流逝的，所以只要通过了 >2020 年的校验，
    // 它几乎肯定会跟上次存的不一样。但为了代码的一致性，我们还是保留比对逻辑。
    time_t now = time(NULL);
    // 2020-01-01 00:00:00 UTC = 1577836800
    if (now > 1577836800) { 
        unsigned long currentSysTime = (unsigned long)now;
        unsigned long savedSysTime = prefs.getULong("sys_time", 0);

        if (savedSysTime != currentSysTime) {
            prefs.putULong("sys_time", currentSysTime);
            // 存时间太频繁了，就不置为 true 了，避免打印太多日志，
            // 或者你可以认为存了时间也算 changed
            // isDataChanged = true; 
        }
    }

    // ==================================================
    // 6. 调试日志
    // ==================================================
    if (isDataChanged) {
        Serial.println("[Storage] Data synced to Flash (Values Changed).");
    } else {
        // 如果你需要确认它真的在运行，可以解开下面这行，否则保持静默
        // Serial.println("[Storage] No changes, skipped writing.");
    }
}