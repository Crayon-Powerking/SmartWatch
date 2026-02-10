#include "controller/AppController.h"
#include "controller/MenuFactory.h"
#include "assets/AppIcons.h"
#include "assets/Lang.h"

// -- 辅助函数 --------------------------------------------------------------------

void AppController::checkSleep() {
    if (isSleeping || (currentApp && currentApp->isKeepAlive()) || AppData.systemConfig.sleepTimeout == 0 || isAlarmRinging) return;
    
    if (millis() - lastActiveTime > (unsigned long)AppData.systemConfig.sleepTimeout * 1000) {
        isSleeping = true;
        display.setPowerSave(1);
    }
}

void AppController::wakeUp() {
    if (isSleeping) {
        isSleeping = false;
        display.setPowerSave(0);
    }
    lastActiveTime = millis();
}

void AppController::checkDayChange() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 50) || timeinfo.tm_year < 120) return;
    int currentDayCode = (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;
    if (currentDayCode != AppData.runtimeCache.lastStepDay) {        
        AppData.runtimeCache.stepCount = 0;
        AppData.runtimeCache.lastStepDay = currentDayCode;
        storage.save();
    }
}

void AppController::checkClock() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 50)) return;
    if (timeinfo.tm_min == lastAlarmMinute) return; 
    lastAlarmMinute = timeinfo.tm_min;
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (!AppData.alarmSlots[i].isUsed || !AppData.alarmSlots[i].isOpen) continue;
        if (AppData.alarmSlots[i].hour == timeinfo.tm_hour && 
            AppData.alarmSlots[i].minute == timeinfo.tm_min) {
            if ((AppData.alarmSlots[i].weekMask >> timeinfo.tm_wday) & 1) {
                if (isSleeping) wakeUp(); // 唤醒屏幕
                isAlarmRinging = true;
                toast.show(STR_ALARM_WARNING[AppData.systemConfig.languageIndex], CONFIG__ALARMING);
            }
        }
    }
}

void AppController::checkWeather() {
    network.requestWeatherUpdate(AppData.userConfig.weather_key, AppData.runtimeCache.currentCityCode);
    timerWeather = millis(); 
}

void AppController::destroyMenuTree() {
    for (auto page : pageList) delete page;
    pageList.clear();
    rootMenu = nullptr;
    menuCtrl.init(nullptr);
}

// -- 按键处理 --------------------------------------------------------------------

bool AppController::processInput() {
    lastActiveTime = millis();

    if (isAlarmRinging) {
        isAlarmRinging = false;
        toast.hide();
        return false;
    }

    if (isSleeping) {
        wakeUp();
        return false; 
    }
    return true; 
}

// 具体的按键逻辑实现
void AppController::onKeySelect() {
    if (!processInput() || currentApp) return;
    if (!inMenuMode) {
        inMenuMode = true;
        menuCtrl.init(rootMenu);
    } else {
        menuCtrl.enter();
    }
}

void AppController::onKeySelectLongPress() {
    if (!processInput() || currentApp) return;
    if (inMenuMode) {
        if (!menuCtrl.back()) inMenuMode = false;
    } else {
        storage.save();
        toast.show(STR_SAVED[AppData.systemConfig.languageIndex]);
    }
}

void AppController::onKeyUp() {
    if (processInput() && !currentApp) {
        inMenuMode ? menuCtrl.prev() : watchFace.onButton(EVENT_KEY_UP);
    }
}

void AppController::onKeyDown() {
    if (processInput() && !currentApp) {
        inMenuMode ? menuCtrl.next() : watchFace.onButton(EVENT_KEY_DOWN);
    }
}

void AppController::bindSystemEvents() {
    // 按键逻辑捆版
    btnSelect.attachClick([this](){ this->onKeySelect(); });
    btnSelect.attachLongPress([this](){ this->onKeySelectLongPress(); });
    btnUp.attachClick([this](){ this->onKeyUp(); });
    btnDown.attachClick([this](){ this->onKeyDown(); });
    
    // 长按连滚逻辑
    auto repeatScroll = [this](bool up) {
        static unsigned long lastTrig = 0;
        if (processInput() && !currentApp && inMenuMode && (millis() - lastTrig > 150)) {
            up ? menuCtrl.prev() : menuCtrl.next();
            lastTrig = millis();
        }
    };
    
    btnUp.attachDuringLongPress([this, repeatScroll](){ repeatScroll(true); });
    btnDown.attachDuringLongPress([this, repeatScroll](){ repeatScroll(false); });
}

// -- 生命周期 (App函数) -----------------------------------------------------------

AppController::AppController() 
    : btnSelect(PIN_BTN_SELECT), btnUp(PIN_BTN_UP), btnDown(PIN_BTN_DOWN) {}

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

    ble.begin("ESP32-Watch");
    ble.setConfigCallback([this](BLEConfigData config) {
        // 1. WiFi 配置
        if (config.hasWifi) {
            strncpy(AppData.userConfig.wifi_ssid, config.ssid, 31);
            strncpy(AppData.userConfig.wifi_pass, config.pass, 63);
            storage.save();
            
            // 收到立即连接，并弹窗提示
            network.connect(config.ssid, config.pass);
            wakeUp();
            toast.show(STR_WIFI[AppData.systemConfig.languageIndex]); 
        }

        // 2. Weather Key
        if (config.hasKey) {
            strncpy(AppData.userConfig.weather_key, config.key, 63);
            storage.save();
            wakeUp();
            toast.show(STR_BLE[AppData.systemConfig.languageIndex]);
        }
    });

    network.begin();
    if (strlen(AppData.userConfig.wifi_ssid) > 0) {
        network.connect(AppData.userConfig.wifi_ssid, AppData.userConfig.wifi_pass);
    }

    // 2. 构建菜单树并绑定按键
    MenuFactory::build(this);
    bindSystemEvents();

    // 3. 初始化计时器
    timerWeather = 0;
    timerSave = millis();
}

void AppController::tick() {
    // -- 1. 系统重载与硬件维护 ----------------------------------------------------
    if (reloadPending) {
        reloadPending = false;
        destroyMenuTree();
        MenuFactory::build(this);
        inMenuMode = true;
        menuCtrl.init(rootMenu);
        return; 
    }

    delay(1); 
    btnSelect.tick();
    btnUp.tick();
    btnDown.tick();
    
    unsigned long now = millis();

    // -- 2. 后台服务 (传感器 / 时间 / 存储 / 网络) ---------------------------------
    
    // IMU 计步与抬腕检测
    static unsigned long lastImuTime = 0;
    if (now - lastImuTime > 30) {
        imu.update(); 
        if(imu.checkStep()) AppData.runtimeCache.stepCount++;
        if (isSleeping && imu.isLiftWrist()) {
            wakeUp();
            lastActiveTime = millis();
        }
        lastImuTime = now;
    }

    // 基础时间服务
    static unsigned long lastTimeCheck = 0;
    if (now - lastTimeCheck > 1000) {
        checkClock();
        checkDayChange();
        lastTimeCheck = now;
    }

    // 检查WiFi 与 蓝牙 连接状态
    static unsigned long lastNetCheck = 0;
    if (now - lastNetCheck > 1000) {
        AppData.isWifiConnected = network.isConnected(); 
        AppData.isBLEConnected = ble.isConnected();
        lastNetCheck = now;
    }

    // 自动保存
    if (now - timerSave > CONFIG_AUTO_SAVE_INTERVAL) {
        storage.save();
        timerSave = now;
    }

    // -- 3. 睡眠门控 --------------------------------------------------------------
    checkSleep();
    if (isSleeping) {
        delay(50); // 息屏降频
        return;    // 阻断渲染与UI逻辑
    }

    // -- 4. 前台服务 (仅亮屏运行) -------------------------------------------------

    if (network.isReady()) { 
        if (timerWeather == 0 || (now - timerWeather > CONFIG_WEATHER_INTERVAL)) {
            checkWeather();
        }
    }

    if (network.isWeatherReady()) {
        WeatherResult res = network.getWeatherResult();
        if (res.success) {
            AppData.runtimeCache.temperature = res.temperature;
            AppData.runtimeCache.weatherCode = res.code;
            AppData.runtimeCache.lastWeatherTime = time(NULL); // 记录更新时间
        }
    }

    // 应用逻辑流
    if (currentApp) {
        if (currentApp->onLoop() != 0) {
            quitApp();
            return; // 退出当前帧
        }
    } else {
        // 菜单动画逻辑
        static unsigned long lastAnimTick = 0;
        if (inMenuMode && (now - lastAnimTick > 10)) {
            menuCtrl.tick(); 
            lastAnimTick = now;
        }
    }

    // -- 5. 渲染管线 (30FPS) ------------------------------------------------------
    
    int targetFps = 30; // 默认系统帧率
    if (currentApp) {
        targetFps = currentApp->getFrameRate();
    }
    unsigned long frameInterval = 1000 / targetFps;
    static unsigned long lastRender = 0;
    if (now - lastRender > frameInterval) { 
        lastRender = now;
        
        if (!currentApp) render(); // 无App时绘制系统界面
        
        toast.draw(&display);      // 绘制弹窗
        display.update();          // 统一提交显存
    }
}

void AppController::startApp(AppBase* app) {
    if (currentApp || !app) return;
    inMenuMode = false;
    currentApp = app;
    currentApp->onRun(this);
}

void AppController::quitApp() {
    if (!currentApp) return;
    currentApp->onExit();
    delete currentApp;
    currentApp = nullptr;
    bindSystemEvents();
    inMenuMode = true; 
    if (menuCtrl.getCurrentPage() == nullptr) menuCtrl.init(rootMenu);
    lastActiveTime = millis();
    display.clear();
    display.update();
}

// -- 渲染绘制 --------------------------------------------------------------------

void AppController::render() {
    if (currentApp) return;
    display.clear();

    if (inMenuMode) {
        MenuPage* currentPage = menuCtrl.getCurrentPage();
        float visualIndex = menuCtrl.getVisualIndex();
        if (currentPage) {
            if (currentPage->layout == LAYOUT_ICON) {
                pageHorizontal.setMenu(currentPage);
                pageHorizontal.setVisualIndex(visualIndex);
                pageHorizontal.draw(&display);
            } else {
                pageVertical.setMenu(currentPage);
                pageVertical.setVisualIndex(visualIndex);
                pageVertical.draw(&display);
            }
        }
    } else {
        watchFace.draw(&display);
    }
}