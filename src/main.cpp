#include <Arduino.h>
#include "AppConfig.h"
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"
#include "model/AppData.h"
#include "service/StorageService.h"

DisplayHAL display;
InputHAL btnSelect(PIN_BTN_SELECT);
InputHAL btnUp(PIN_BTN_UP);
InputHAL btnDown(PIN_BTN_DOWN);
StorageService storage;

// --- 核心修改：改为 5 分钟保存一次 ---
unsigned long lastSaveTime = 0;
// 5分钟 * 60秒 * 1000毫秒 = 300,000
const unsigned long SAVE_INTERVAL = 300000; 

void updateView() {
    display.clear();
    
    // 顶部状态栏
    display.drawText(80, 10, "WiFi"); 

    if (AppData.currentScreen == 0) {
        display.drawText(10, 20, "--- Steps ---");
        char buf[32];
        sprintf(buf, "%d", AppData.stepCount);
        display.drawText(40, 50, buf);
    } else if (AppData.currentScreen == 1) {
        display.drawText(10, 20, "--- Battery ---");
        char buf[32];
        sprintf(buf, "%d %%", AppData.batteryLevel);
        display.drawText(40, 50, buf);
    }
    
    display.update();
}

void setup() {
    Serial.begin(115200);
    
    // 硬件初始化
    display.begin();
    btnSelect.begin();
    btnUp.begin();
    btnDown.begin();
    
    // 存储初始化：开机读取一次历史数据
    storage.begin();
    storage.load(); 

    // --- 绑定按键 (注意：按键里不执行 save) ---
    btnSelect.attachClick([](){
        AppData.currentScreen = (AppData.currentScreen + 1) % 2;
        updateView();
    });

    btnUp.attachClick([](){
        if (AppData.currentScreen == 0) {
            AppData.stepCount++;
        } else {
            AppData.batteryLevel++;
        }
        updateView();
    });

    btnDown.attachClick([](){
        if (AppData.currentScreen == 0) {
            if(AppData.stepCount > 0) AppData.stepCount--;
        } else {
            AppData.batteryLevel--;
        }
        updateView();
    });
    
    // 长按清零：这是一个重大操作，所以我们保留“立即强制保存”
    // 用户很少会清零，所以这里强制写一次 Flash 是合理的
    btnDown.attachLongPress([](){
        AppData.stepCount = 0;
        updateView();
        storage.save(); 
        Serial.println("Force Saved (Reset)");
    });

    updateView();
    Serial.println("System Ready. Auto-save set to 5 min.");
}

void loop() {
    // 1. 驱动心跳
    btnSelect.tick();
    btnUp.tick();
    btnDown.tick();

    // 2. 定时自动保存策略 (Auto-Save)
    // 只有当开机运行时间超过 5 分钟，且距离上次保存超过 5 分钟时才会触发
    if (millis() - lastSaveTime > SAVE_INTERVAL) {
        storage.save(); // 内部还有一层判断：数据变了才写
        lastSaveTime = millis();
    }
    
    delay(10);
}