#include "controller/AppController.h"
#include "controller/MenuFactory.h"
#include "assets/AppIcons.h"
#include "assets/Lang.h"

// 引用外部对象
extern DisplayHAL display;
extern InputHAL btnSelect;
extern InputHAL btnUp;
extern InputHAL btnDown;

AppController::AppController() {}

void AppController::begin() {
    // 1. 启动硬件和服务
    storage.begin();
    storage.load();
    network.begin(WIFI_SSID, WIFI_PASS);

    // 2. 核心：构建菜单树
    MenuFactory::build(this);
    
    // [SELECT 单击]
    btnSelect.attachClick([this](){
        if (!inMenuMode) {
            inMenuMode = true;
            if (rootMenu && !rootMenu->items.empty()) {
                rootMenu->selectedIndex = rootMenu->items.size() / 2;
            }
            menuCtrl.init(rootMenu);
        } else {
            menuCtrl.enter();
        }
    });

    // [SELECT 长按]
    btnSelect.attachLongPress([this](){
        if (inMenuMode) {
            if (!menuCtrl.back()) inMenuMode = false;
        } else {
            storage.save();
            toast.show(STR_SAVED[AppData.languageIndex]);
        }
    });

    // [UP 连发]
    btnUp.attachDuringLongPress([this](){
        if (inMenuMode) {
            static unsigned long lastTrig = 0;
            if (millis() - lastTrig > 150) {
                menuCtrl.prev();
                lastTrig = millis();
            }
        }
    });

    // [UP 单击]
    btnUp.attachClick([this](){
        if (inMenuMode) menuCtrl.prev();
        else {
            watchFace.onButton(EVENT_KEY_UP);
        }
    });

    // [DOWN 连发]
    btnDown.attachDuringLongPress([this](){
        if (inMenuMode) {
            static unsigned long lastTrig = 0;
            if (millis() - lastTrig > 150) {
                menuCtrl.next();
                lastTrig = millis();
            }
        }
    });

    // [DOWN 单击]
    btnDown.attachClick([this](){
        if (inMenuMode) menuCtrl.next();
        else {
            watchFace.onButton(EVENT_KEY_DOWN);
        }
    });

    // 4. 初始化计时器
    timerWeather = 0;
    timerSave = millis();
}

// ------------------------------------------------------------
// 【成熟方案】彻底销毁所有页面，杜绝内存泄漏
// ------------------------------------------------------------
void AppController::destroyMenuTree() {
    for (auto page : pageList) {
        delete page;
    }
    pageList.clear();
    rootMenu = nullptr;
    menuCtrl.init(nullptr);
}

void AppController::tick() {
    delay(1); 

    network.tick();
    AppData.isWifiConnected = network.isConnected();
    static unsigned long wifiConnectTime = 0;
    static bool lastWifiState = false;
    if (AppData.isWifiConnected && !lastWifiState) {
        wifiConnectTime = millis();
    }
    lastWifiState = AppData.isWifiConnected;

    unsigned long now = millis();

    // 菜单动画
    static unsigned long lastAnimTick = 0;
    if (inMenuMode && (now - lastAnimTick > 10)) {
        menuCtrl.tick(); 
        lastAnimTick = now;
    }

    // 智能天气
    if (network.isConnected()) {
        if (now - wifiConnectTime > 2000) { 
            if (timerWeather == 0 || (now - timerWeather > 3600000)) {
                checkWeather();
            }
        }
    }
    
    // 步数检查
    checkDayChange();
    
    // 自动保存
    if (now - timerSave > 1800000) {
        storage.save();
        timerSave = now;
    }

    // 渲染
    static unsigned long lastRender = 0;
    if (now - lastRender > 33) { 
        render();
        lastRender = now;
    }
}

void AppController::checkWeather() {
    if (weatherTaskHandle != NULL) return; 
    xTaskCreate(weatherTask, "WeatherTask", 4096, this, 1, &weatherTaskHandle);
}

void AppController::weatherTask(void* parameter) {
    AppController* app = (AppController*)parameter;
    WeatherResult res = app->network.fetchWeather(WEATHER_KEY, WEATHER_CITY);
    
    if (res.success) {
        AppData.temperature = res.temperature;
        AppData.weatherCode = res.code;
        AppData.lastWeatherTime = time(NULL);
        app->timerWeather = millis(); 
    } else {
        app->timerWeather = millis() - 3600000 + 60000; 
    }
    app->weatherTaskHandle = NULL; 
    vTaskDelete(NULL);
}

void AppController::checkDayChange() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 0)) return;
    if (timeinfo.tm_year < (2020 - 1900)) return;

    int currentDayCode = (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;

    if (AppData.lastStepDayCode == 0) {
        AppData.lastStepDayCode = currentDayCode;
        storage.save();
        return;
    }

    if (currentDayCode != AppData.lastStepDayCode) {        
        AppData.stepCount = 0;
        AppData.lastStepDayCode = currentDayCode;
        storage.save();
    }
}

void AppController::render() {
    display.clear(); // 清屏

    if (inMenuMode) {
        // 1. 获取当前要显示的页面
        MenuPage* currentPage = menuCtrl.getCurrentPage();
        
        // 2. 获取动画过渡值
        float visualIndex = menuCtrl.getVisualIndex();
        
        if (currentPage) {
            // === 核心判断：根据页面类型选择画师 ===
            if (currentPage->layout == LAYOUT_ICON) {
                // A. 横向画师 (一级菜单)
                pageHorizontal.setMenu(currentPage);
                pageHorizontal.setVisualIndex(visualIndex);
                pageHorizontal.draw(&display);
            } 
            else {
                // B. 纵向画师 (二级菜单)
                pageVertical.setMenu(currentPage);
                pageVertical.setVisualIndex(visualIndex);
                pageVertical.draw(&display);
            }
        }
    } else {
        // 3. 不在菜单模式 -> 画表盘
        watchFace.draw(&display);
    }

    // 绘制全局 Toast (弹窗)
    toast.draw(&display);
    
    // 推送显存
    display.update();
}