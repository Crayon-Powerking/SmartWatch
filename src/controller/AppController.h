#pragma once
#include "AppConfig.h"
#include "model/AppData.h"
#include "model/MenuTypes.h"
#include <vector>
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"
#include "service/NetworkService.h"
#include "service/StorageService.h"

// 引入视图
#include "view/PageWatchFace.h"
#include "view/PageHorizontalMenu.h" 
#include "view/PageVerticalMenu.h"   // 【新增】 引入垂直列表视图
#include "view/SystemToast.h"
#include "controller/MenuController.h"

class MenuFactory;
class SettingsBuilder;
class GamesBuilder;

class AppController {
    
    friend class MenuFactory;
    friend class SettingsBuilder;
    friend class GamesBuilder;

public:
    AppController(); 
    void begin();
    void tick();

private:
    SystemToast toast;

    // --- 内部私有函数 ---
    void destroyMenuTree();       // 销毁旧菜单
    void checkWeather();          // 智能检查天气
    void render();                // 统一渲染入口
    void checkDayChange();        // 检查日期变化，重置步数
    
    // --- 核心模块 ---
    NetworkService network;
    StorageService storage;
    MenuController menuCtrl; // 菜单逻辑核心

    // --- 视图渲染器 ---
    PageWatchFace watchFace;       // 表盘
    PageHorizontalMenu pageHorizontal; // 【重命名】 横向画师 (原 menuView)
    PageVerticalMenu   pageVertical;   // 【新增】   纵向画师

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

    // 【核心新增】页面垃圾回收站
    // 用于追踪所有 new 出来的 MenuPage，确保 destroy 时能全部删干净
    std::vector<MenuPage*> pageList; 

    // 辅助函数：创建一个新页面并自动注册到垃圾回收站
    MenuPage* createPage(const char* title, MenuLayout layout = LAYOUT_LIST) {
        MenuPage* p = new MenuPage(title, layout);
        pageList.push_back(p);
        return p;
    }
};