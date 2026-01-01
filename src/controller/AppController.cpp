#include "controller/AppController.h"
#include "assets/AppIcons.h"


// 引用外部对象
extern DisplayHAL display;
extern InputHAL btnSelect;
extern InputHAL btnUp;
extern InputHAL btnDown;

AppController::AppController() {}

void AppController::begin() {
    Serial.println("[App] Controller Starting...");

    // 1. 启动硬件和服务
    storage.begin();
    storage.load();
    network.begin(WIFI_SSID, WIFI_PASS);

    // 2. 核心：构建菜单树 (一次性造好所有菜单)
    buildMenuTree();

    // 3. 绑定按键
    // [SELECT 单击]
    btnSelect.attachClick([this](){
        if (!inMenuMode) {
            // 表盘 -> 进菜单
            inMenuMode = true;
            menuCtrl.init(rootMenu); // 永远从一级菜单开始
        } else {
            // 菜单 -> 执行空函数
            menuCtrl.enter();
        }
        render();
    });

    // [SELECT 长按]
    btnSelect.attachLongPress([this](){
        if (inMenuMode) {
            // 菜单 -> 强制退回表盘 (一级菜单退无可退，直接由 back() 返回 false 触发)
            if (!menuCtrl.back()) inMenuMode = false;
        } else {
            storage.save(); // 表盘 -> 保存
        }
        render();
    });

    // [UP / DOWN]
    btnUp.attachClick([this](){
        if (inMenuMode) menuCtrl.prev();
        else pageMgr.dispatchInput(EVENT_KEY_UP);
        render();
    });

    btnDown.attachClick([this](){
        if (inMenuMode) menuCtrl.next();
        else pageMgr.dispatchInput(EVENT_KEY_DOWN);
        render();
    });

    // 4. 初始化计时器 (补全了这里)
    timerWeather = 0;       // 立即触发天气更新
    timerSave = millis();   // 从现在开始倒计时自动保存
}

// ==========================================================
// 核心修改：批量生产一级菜单
// ==========================================================
void AppController::buildMenuTree() {
    // 创建根菜单 (一级菜单)
    rootMenu = new MenuPage("Home");

    // --- 批量生产开始 ---

    // Item 1: 设置 (测试批量)
    rootMenu->add("Setting", icon_setting, [](){ 
        Serial.println("[Menu] Clicked Setting (TODO)"); 
    });
    // Item 2: 天气
    rootMenu->add("Weather", icon_weather, [this](){ 
        Serial.println("[Menu] Clicked Weather - Syncing...");
        checkWeather(); 
    });
    // Item 3: 闹钟
    rootMenu->add("Alarm", icon_alarm, [](){ 
        Serial.println("[Menu] Clicked Alarm (TODO)"); 
    });
    // Item 4: 手电筒
    rootMenu->add("Flashlight", icon_flashlight, [](){ 
        Serial.println("[Menu] Clicked Flashlight (TODO)"); 
    });
    // Item 5: 关于
    rootMenu->add("About", icon_information, [](){ 
        Serial.println("[Menu] Clicked About (TODO)"); 
    });
    if (!rootMenu->items.empty()) {
        rootMenu->selectedIndex = rootMenu->items.size() / 2;
    }
    // 你可以继续复制粘贴 add... 
    // 观察屏幕底部的滚动条会自动根据数量调整宽度
}

void AppController::tick() {
    // 1. 服务心跳
    network.tick();
    AppData.isWifiConnected = network.isConnected();
    unsigned long now = millis();

    // 智能天气
    if (network.isConnected()) {
        if (timerWeather == 0 || (now - timerWeather > 3600000)) {
            checkWeather();
        }
    }

    // 自动保存
    if (now - timerSave > 300000) {
        storage.save();
        timerSave = now;
    }

    // 屏幕刷新
    static unsigned long lastRender = 0;
    if (now - lastRender > 40) {
        render();
        lastRender = now;
    }
}

void AppController::checkWeather() {
    // 强制拉取
    WeatherResult res = network.fetchWeather(WEATHER_KEY, WEATHER_CITY);
    
    if (res.success) {
        // 更新数据模型
        AppData.temperature = res.temperature;
        AppData.weatherCode = res.code;
        AppData.lastWeatherTime = time(NULL);
        
        // 标记：更新成功了！重置计时器
        timerWeather = millis(); 
        
        // 立即保存到 Flash，这样下次开机就是新的了
        storage.save(); 
        Serial.println("[App] Weather Updated & Saved.");
    } else {
        // 失败了？那 1 分钟后再试，别死循环刷爆 API
        timerWeather = millis() - 3600000 + 60000; 
    }
}

void AppController::render() {
    display.clear();
    if (inMenuMode) {
        // 渲染菜单 (用新的横向视图)
        // View 只负责画，Data 从 Controller 拿
        menuView.setMenu(menuCtrl.getCurrentPage());
        menuView.draw(&display);
    } else {
        // 渲染表盘 (旧的)
        pageMgr.render(&display);
    }
    display.update();
}
