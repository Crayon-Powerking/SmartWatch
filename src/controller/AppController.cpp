#include "controller/AppController.h"
#include "controller/MenuFactory.h"
#include "assets/AppIcons.h"
#include "assets/Lang.h"

AppController::AppController() : btnSelect(PIN_BTN_SELECT), btnUp(PIN_BTN_UP),btnDown(PIN_BTN_DOWN) {}

void AppController::wakeUp() {
    if (isSleeping) {
        isSleeping = false;
        display.setPowerSave(0);
    }
}

void AppController::onButtonEvent() {
    lastActiveTime = millis();
    if (isSleeping) {
        wakeUp();
    }
}

void AppController::checkSleep() {
    if (isSleeping) return;
    // 如果当前在运行 App，则重置计时器并返回
    if (currentApp && currentApp->isKeepAlive()) {
        lastActiveTime = millis(); 
        return;
    }
    // 如果设置为 "永不息屏"，直接返回
    if (AppData.systemConfig.sleepTimeout == 0) return;

    // 检查是否超时
    unsigned long timeoutMs = AppData.systemConfig.sleepTimeout * 1000;
    
    if (millis() - lastActiveTime > timeoutMs) {
        isSleeping = true;
        display.setPowerSave(1);
    }
}

// 绑定系统按键事件到菜单控制器
void AppController::bindSystemEvents() {
    // [SELECT 单击]
    btnSelect.attachClick([this](){
        this->onButtonEvent();
        if (currentApp) return; 
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
        this->onButtonEvent();
        if (currentApp) return; 
        if (inMenuMode) {
            if (!menuCtrl.back()) inMenuMode = false;
        } else {
            storage.save();
            toast.show(STR_SAVED[AppData.systemConfig.languageIndex]);
        }
    });

    // [UP 单击]
    btnUp.attachClick([this](){
        this->onButtonEvent();
        if (currentApp) return; 
        if (inMenuMode) menuCtrl.prev();
        else watchFace.onButton(EVENT_KEY_UP);
    });

    // [UP 连发]
    btnUp.attachDuringLongPress([this](){
        this->onButtonEvent();
        if (currentApp) return; 
        if (inMenuMode) {
            static unsigned long lastTrig = 0;
            if (millis() - lastTrig > 150) {
                menuCtrl.prev();
                lastTrig = millis();
            }
        }
    });

    // [DOWN 单击]
    btnDown.attachClick([this](){
        this->onButtonEvent();
        if (currentApp) return; 
        if (inMenuMode) menuCtrl.next();
        else watchFace.onButton(EVENT_KEY_DOWN);
    });

    // [DOWN 连发]
    btnDown.attachDuringLongPress([this](){
        this->onButtonEvent();
        if (currentApp) return; 
        if (inMenuMode) {
            static unsigned long lastTrig = 0;
            if (millis() - lastTrig > 150) {
                menuCtrl.next();
                lastTrig = millis();
            }
        }
    });  
}

void AppController::begin() {
    lastActiveTime = millis();
    // 1. 启动硬件和服务
    storage.begin();
    storage.load();
    display.begin();
    btnSelect.begin();
    btnUp.begin();
    btnDown.begin();
    imu.begin();
    network.begin(WIFI_SSID, WIFI_PASS);

    // 2. 构建菜单树
    MenuFactory::build(this);
    
    // 3. 调用事件绑定
    bindSystemEvents();

    // 4. 初始化计时器
    timerWeather = 0;
    timerSave = millis();
}

void AppController::destroyMenuTree() {
    for (auto page : pageList) {
        delete page;
    }
    pageList.clear();
    rootMenu = nullptr;
    menuCtrl.init(nullptr);
}

// 启动 App
void AppController::startApp(AppBase* app) {
    if (currentApp) return; // 防止重复启动
    
    inMenuMode = false; // 退出菜单模式
    currentApp = app;
    
    // 调用 App 的生命周期 setup，并传入 controller 指针
    if (currentApp) {
        currentApp->onRun(this);
    }
}

// 退出 App
void AppController::quitApp() {
    if (!currentApp) return;

    // 1. 清理
    currentApp->onExit();
    delete currentApp;
    currentApp = nullptr;

    // 2. 恢复系统的按键接管
    bindSystemEvents();

    // 3. 恢复界面
    inMenuMode = true; 
    menuCtrl.init(rootMenu);
    
    display.clear();
    display.update();
}

void AppController::tick() {

    if (reloadPending) {
        reloadPending = false;
        destroyMenuTree();
        MenuFactory::build(this); // 重建
        
        // 重置到主菜单
        inMenuMode = true;
        menuCtrl.init(rootMenu);
        return; 
    }

    delay(1); 

    btnSelect.tick();
    btnUp.tick();
    btnDown.tick();
    
    unsigned long now = millis();
    static unsigned long lastImuTime = 0;
    // 限制频率：每 100ms 读取一次传感器 (10Hz)
    if (now - lastImuTime > 100) {
        imu.update(); 
        if (isSleeping && imu.isLiftWrist()) {
            wakeUp();                 // 唤醒屏幕
            lastActiveTime = millis();
        }
        lastImuTime = now;
    }

    checkSleep();
    if (isSleeping) {
        delay(50); 
        return; 
    }

    if (currentApp) {
        // 执行 App 循环，并检查返回值
        int status = currentApp->onLoop();
        if (status != 0) {
            quitApp();
        } else {
            network.tick(); // 保持网络心跳
        }
        return;
    }

    // --- 以下是常规系统模式 (表盘/菜单) ---

    network.tick();
    AppData.isWifiConnected = network.isConnected();
    static unsigned long wifiConnectTime = 0;
    static bool lastWifiState = false;
    if (AppData.isWifiConnected && !lastWifiState) {
        wifiConnectTime = millis();
    }
    lastWifiState = AppData.isWifiConnected;

    now = millis();
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

// 检查并更新天气数据（在后台任务中执行）
void AppController::checkWeather() {
    // 防止重复创建任务
    if (weatherTaskHandle != NULL) return; 
    
    // 创建 FreeRTOS 任务获取天气
    // 堆栈大小 4096, 优先级 1
    xTaskCreate(weatherTask, "WeatherTask", 4096, this, 1, &weatherTaskHandle);
}

void AppController::weatherTask(void* parameter) {
    AppController* app = (AppController*)parameter;
    WeatherResult res = app->network.fetchWeather(WEATHER_KEY, AppData.runtimeCache.currentCityCode);
    if (res.success) {
        AppData.runtimeCache.temperature = res.temperature;
        AppData.runtimeCache.weatherCode = res.code;
        AppData.runtimeCache.lastWeatherTime = time(NULL);
        app->timerWeather = millis(); 
        app->storage.save();
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

    if (AppData.runtimeCache.lastStepDay == 0) {
        AppData.runtimeCache.lastStepDay = currentDayCode;
        storage.save();
        return;
    }

    if (currentDayCode != AppData.runtimeCache.lastStepDay) {        
        AppData.runtimeCache.stepCount = 0;
        AppData.runtimeCache.lastStepDay = currentDayCode;
        storage.save();
    }
}

void AppController::render() {
    if (currentApp) return;

    display.clear(); // 清屏

    if (inMenuMode) {
        MenuPage* currentPage = menuCtrl.getCurrentPage();
        float visualIndex = menuCtrl.getVisualIndex();
        
        if (currentPage) {
            if (currentPage->layout == LAYOUT_ICON) {
                pageHorizontal.setMenu(currentPage);
                pageHorizontal.setVisualIndex(visualIndex);
                pageHorizontal.draw(&display);
            } 
            else {
                pageVertical.setMenu(currentPage);
                pageVertical.setVisualIndex(visualIndex);
                pageVertical.draw(&display);
            }
        }
    } else {
        watchFace.draw(&display);
    }

    toast.draw(&display);
    display.update();
}

void AppController::forceWeatherUpdate() {
    // 只有在联网且当前没有正在进行的天气任务时，才执行
    if (network.isConnected() && weatherTaskHandle == NULL) {
        checkWeather(); // 立即启动后台任务
    }
}