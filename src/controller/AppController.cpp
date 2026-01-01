#include "controller/AppController.h"
#include "assets/AppIcons.h"

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
    buildMenuTree();
    
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
        }
        // render(); // <--- 删除！
    });

    // [UP 连发] (保持你的 150ms 限流逻辑，非常正确)
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
            watchFace.onButton(EVENT_KEY_UP);       // <--- 直接发给表盘 (虽然目前表盘没逻辑，但保留接口)
        }
    });

    // [DOWN 连发] (保持 150ms 限流)
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
            watchFace.onButton(EVENT_KEY_DOWN);       // <--- 直接发给表盘
        }
    });

    // 4. 初始化计时器
    timerWeather = 0;
    timerSave = millis();
}

void AppController::buildMenuTree() {
    rootMenu = new MenuPage("Home");

    rootMenu->add("Calendar", icon_calendar, [this](){ 
        // TODO
    });
    rootMenu->add("Weather", icon_weather, [this](){ 
        // TODO
    });
    rootMenu->add("Alarm", icon_alarm, [](){ 
        // TODO
    });
    rootMenu->add("Setting", icon_setting, [](){ 
        // TODO: Enter Setting
    });
    rootMenu->add("Tool", icon_tool, [](){ 
        // TODO
    });
    rootMenu->add("Game", icon_game, [this](){ 
        // TODO
    });
    rootMenu->add("About", icon_information, [](){ 
        // TODO
    });
    
    if (!rootMenu->items.empty()) {
        rootMenu->selectedIndex = rootMenu->items.size() / 2;
    }
}

void AppController::tick() {
    // ------------------------------------------------------------
    // 【关键】系统让步：防止 WiFi 任务饿死重启
    // ------------------------------------------------------------
    delay(1); 

    // 1. 服务心跳
    network.tick();
    bool currentWifiState = network.isConnected();

    static unsigned long wifiConnectTime = 0;
    static bool lastWifiState = false;
    if (AppData.isWifiConnected && !lastWifiState) {
        wifiConnectTime = millis();
    }
    lastWifiState = AppData.isWifiConnected;

    unsigned long now = millis();

    // 2. 菜单动画逻辑 (10ms 保持丝滑计算)
    static unsigned long lastAnimTick = 0;
    if (inMenuMode && (now - lastAnimTick > 10)) {
        menuCtrl.tick(); 
        lastAnimTick = now;
    }

    // 3. 智能天气 (优化版：避让 NTP)
    if (network.isConnected()) {
        // 【核心修改】增加一个条件：连网时间必须超过 2秒 (2000ms)
        // 这样给 NTP 留出 2秒 的独占时间，防止 HTTP 请求把 NTP 挤掉
        if (now - wifiConnectTime > 2000) { 
            if (timerWeather == 0 || (now - timerWeather > 3600000)) {
                checkWeather();
            }
        }
    }
    checkDayChange();
    // 4. 自动保存 (5分钟)
    if (now - timerSave > 1800000) {
        storage.save();
        timerSave = now;
    }

    // 5. 渲染 (33ms)
    static unsigned long lastRender = 0;
    if (now - lastRender > 33) { 
        render();
        lastRender = now;
    }
}

void AppController::checkWeather() {
    // 如果任务已经在运行，不要重复创建
    if (weatherTaskHandle != NULL) {
        Serial.println("[App] Weather task is running, skip.");
        return; 
    }    
    // 创建后台任务运行 fetchWeather
    xTaskCreate(
        weatherTask,   
        "WeatherTask", 
        4096,          // 栈大小
        this,          // 传入 this 指针
        1,             // 优先级
        &weatherTaskHandle 
    );
}
// 静态任务函数 (在后台跑，不阻塞主循环)
void AppController::weatherTask(void* parameter) {
    AppController* app = (AppController*)parameter;
    WeatherResult res = app->network.fetchWeather(WEATHER_KEY, WEATHER_CITY);
    
    if (res.success) {
        AppData.temperature = res.temperature;
        AppData.weatherCode = res.code;
        AppData.lastWeatherTime = time(NULL);
        
        // 更新成功，重置定时器
        app->timerWeather = millis(); 
        
        // 注意：多线程下尽量只更新内存，把 Flash 写入留给主循环的自动保存
    } else {
        // 失败重试 (1分钟后)
        app->timerWeather = millis() - 3600000 + 60000; 
    }
    // 任务自杀
    app->weatherTaskHandle = NULL; 
    vTaskDelete(NULL);
}

// ------------------------------------------------------------
// 【新增逻辑】每日步数清零
// ------------------------------------------------------------
void AppController::checkDayChange() {
    struct tm timeinfo;
    // 1. 必须确保时间已经同步 (大于2020年)
    if (!getLocalTime(&timeinfo, 0)) return;
    if (timeinfo.tm_year < (2020 - 1900)) return;

    // 2. 生成当前日期的唯一代码: YYYYMMDD
    // tm_year 是从1900开始的，tm_mon 是0-11
    int currentDayCode = (timeinfo.tm_year + 1900) * 10000 + (timeinfo.tm_mon + 1) * 100 + timeinfo.tm_mday;

    // 3. 如果是第一次运行 (0)，直接同步日期，不重置步数 (防止意外清零)
    if (AppData.lastStepDayCode == 0) {
        AppData.lastStepDayCode = currentDayCode;
        return;
    }

    // 4. 核心检查：如果当前日期 != 上次保存日期
    if (currentDayCode != AppData.lastStepDayCode) {
        Serial.printf("[App] New Day! Reset Steps. Old: %d, New: %d\n", AppData.lastStepDayCode, currentDayCode);
        
        // 清零步数
        AppData.stepCount = 0;
        // 更新日期
        AppData.lastStepDayCode = currentDayCode;
        // 立即保存一次
        storage.save();
    }
}

void AppController::render() {
    display.clear();
    if (inMenuMode) {
        menuView.setMenu(menuCtrl.getCurrentPage());
        menuView.setVisualIndex(menuCtrl.getVisualIndex());
        menuView.draw(&display);
    } else {
        watchFace.draw(&display);
    }
    display.update();
}