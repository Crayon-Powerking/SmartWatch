#pragma once
#include "AppConfig.h"
#include "model/AppData.h"
#include "model/MenuTypes.h"
 
// 引入各层模块
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"
#include "service/NetworkService.h"
#include "service/StorageService.h"

// 引入视图
#include "view/PageWatchFace.h"
#include "view/PageHorizontalMenu.h" // 负责菜单 (新增的横向视图)
#include "controller/MenuController.h" // 负责菜单逻辑

class AppController {
public:
    AppController(); 
    void begin();
    void tick();

private:
    // --- 内部私有函数 ---
    void buildMenuTree();         // 构建菜单结构
    void checkWeather();          // 智能检查天气
    void render();                // 统一渲染入口
    void checkDayChange();        // 检查日期变化，重置步数
    
    // --- 核心模块 ---
    NetworkService network;
    StorageService storage;
    MenuController menuCtrl; // 菜单逻辑核心

    // --- 视图渲染器 ---
    PageWatchFace watchFace;       // 表盘渲染器
    PageHorizontalMenu menuView;   // 菜单渲染器 (新的横向)

    // --- 菜单数据 (Model) ---
    MenuPage* rootMenu = nullptr;
    MenuPage* toolsMenu = nullptr;

    // --- 状态标志 ---
    bool inMenuMode = false;      // 当前是在看菜单吗？
    unsigned long timerWeather = 0;
    unsigned long timerSave = 0;

    // 【新增】后台任务相关
    TaskHandle_t weatherTaskHandle = NULL; // 任务句柄
    static void weatherTask(void* parameter); // 静态任务函数
};