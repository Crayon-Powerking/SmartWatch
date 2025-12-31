#include <Arduino.h>
#include "AppConfig.h"
#include "model/AppData.h"
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"
#include "view/PageManager.h"
#include "service/StorageService.h"
#include "service/NetworkService.h"

// 实例化模块
DisplayHAL display;
InputHAL btnSelect(PIN_BTN_SELECT);
InputHAL btnUp(PIN_BTN_UP);
InputHAL btnDown(PIN_BTN_DOWN);
StorageService storage;
PageManager pageMgr;
NetworkService network;

unsigned long timerSave = 0;    // 用于自动保存步数和时间
unsigned long timerWeather = 0; // 用于天气更新检查

void setup() {
    Serial.begin(115200);
    // 1. 初始化硬件
    display.begin();
    btnSelect.begin();
    btnUp.begin();
    btnDown.begin();
    
    // 2. 初始化存储并读取旧数据 (包括上次的天气缓存)
    storage.begin();
    storage.load(); 
    
    // 3. 启动网络 (后台连接)
    network.begin(WIFI_SSID, WIFI_PASS);

    // 4. 按键回调
    btnSelect.attachClick([](){ pageMgr.switchNext(); pageMgr.render(&display); });
    btnUp.attachClick([](){ pageMgr.dispatchInput(EVENT_KEY_UP); pageMgr.render(&display); });
    btnDown.attachClick([](){ pageMgr.dispatchInput(EVENT_KEY_DOWN); pageMgr.render(&display); });
    
    // 初始化计时器
    timerSave = millis();
    timerWeather = millis();

    // 5. 初始渲染
    pageMgr.render(&display);
    Serial.println("System Ready.");
}

void loop() {
    // --- 1. 驱动底层心跳 ---
    btnSelect.tick();
    btnUp.tick();
    btnDown.tick();
    network.tick(); // 维护 WiFi 连接和 NTP 状态

    unsigned long now = millis();

    // --- 2. 策略 A: 定时自动保存 (常规任务) ---
    // 只要时间间隔到了，就存一次（StorageService 内部会负责比对数据，没变就不写 Flash）
    if (now - timerSave > CONFIG_AUTO_SAVE_INTERVAL) {
        Serial.println("[Main] Auto-save timer triggered.");
        storage.save(); 
        
        timerSave = now; // 重置计时器
    }

    // --- 3. 策略 B: 智能天气更新 (突发高优先级任务) ---
    if (network.isConnected()) {
        // 检查是否到了更新天气的时间
        // (timerWeather == 0 是为了应对 millis() 溢出的极端边界情况，通常可忽略)
        if (now - timerWeather > CONFIG_WEATHER_INTERVAL) {
            
            // 使用 AppConfig.h 里的 Key 和 City
            WeatherResult res = network.fetchWeather(WEATHER_KEY, WEATHER_CITY);
            
            if (res.success) {
                // 更新内存数据
                AppData.temperature = res.temperature;
                AppData.weatherCode = res.code;
                AppData.lastWeatherTime = time(NULL); 
                
                // --- 关键优化逻辑 ---
                // 因为天气数据刚更新，很珍贵，所以打破常规，立刻保存
                storage.save(); 
                
                // 重置自动保存计时器！！
                // 既然刚刚已经存过了，那么原来的“5分钟倒计时”就可以重新开始了
                // 这样避免了“刚存完天气，过了10秒自动保存又来存一次”的浪费
                timerSave = now; 
                
                // 更新天气计时器
                timerWeather = now;
            } else {
                // 如果失败，可以选择不更新 timerWeather，这样下一次 loop 会立即重试
                // 或者更新它，让它过一小时再试（防止断网时死循环请求）
                // 这里选择：稍微推迟 1 分钟再试，避免死循环刷爆日志
                // timerWeather = now - CONFIG_WEATHER_INTERVAL + 60000; 
            }
        }
    }

    // --- 4. 屏幕刷新 (UI 渲染) ---
    // 保持 500ms 刷新率，确保时间秒针流畅跳动
    static unsigned long lastRender = 0;
    if (now - lastRender > 500) {
        pageMgr.render(&display);
        lastRender = now;
    }

    delay(10); // 短暂休眠，让出 CPU 给 WiFi 协议栈
}