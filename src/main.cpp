#include <Arduino.h>
#include "AppConfig.h"
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

// 全局网络服务实例 (PageTime.h 里用了 extern 引用它)
NetworkService network;

// --- WiFi 配置 (请修改这里！) ---
const char* WIFI_SSID = "8513";     // 你的 WiFi 名字
const char* WIFI_PASS = "ai123456"; // 你的 WiFi 密码

unsigned long lastSaveTime = 0;
const unsigned long SAVE_INTERVAL = 300000; 

void setup() {
    Serial.begin(115200);
    
    display.begin();
    btnSelect.begin();
    btnUp.begin();
    btnDown.begin();
    storage.begin();
    storage.load();

    // --- 启动联网 ---
    // ESP32 会在后台去连接，不会阻塞这里
    network.begin(WIFI_SSID, WIFI_PASS);

    // 绑定按键
    btnSelect.attachClick([](){
        pageMgr.switchNext();
        pageMgr.render(&display);
    });

    btnUp.attachClick([](){
        pageMgr.dispatchInput(EVENT_KEY_UP);
        pageMgr.render(&display);
    });

    btnDown.attachClick([](){
        pageMgr.dispatchInput(EVENT_KEY_DOWN);
        pageMgr.render(&display);
    });
    
    // 长按保存
    btnDown.attachLongPress([](){
        AppData.stepCount = 0;
        pageMgr.render(&display);
        storage.save();
    });

    pageMgr.render(&display);
    Serial.println("System Ready. Connecting to WiFi...");
}

void loop() {
    // 1. 所有的 tick 都要跑
    btnSelect.tick();
    btnUp.tick();
    btnDown.tick();
    
    // 新增：网络服务也要心跳 (检查连接状态)
    network.tick();

    // 2. 自动保存
    if (millis() - lastSaveTime > SAVE_INTERVAL) {
        storage.save();
        lastSaveTime = millis();
    }
    
    // 3. 动态刷新 (关键修改！)
    // 因为时间每秒都在变，我们不能只靠按键来触发刷新了。
    // 我们需要在 loop 里不断刷新屏幕，才能看到秒针闪烁。
    // 为了不闪瞎眼，我们可以每 500ms 刷新一次。
    static unsigned long lastRender = 0;
    if (millis() - lastRender > 500) {
        pageMgr.render(&display);
        lastRender = millis();
    }

    delay(10);
}