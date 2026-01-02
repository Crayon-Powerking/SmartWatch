#pragma once
#include "AppConfig.h"
#include "model/AppData.h"
#include "model/MenuTypes.h"
#include "model/AppBase.h" // 【新增】 引入 App 基类
#include <vector>
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"
#include "service/NetworkService.h"
#include "service/StorageService.h"

// 引入视图
#include "view/PageWatchFace.h"
#include "view/PageHorizontalMenu.h" 
#include "view/PageVerticalMenu.h"
#include "view/SystemToast.h"
#include "controller/MenuController.h"

class MenuFactory;
class SettingsBuilder;
class GamesBuilder;

class AppController {
    
    friend class MenuFactory;
    friend class SettingsBuilder;
    friend class GamesBuilder;
    friend class AppBase; 

public:
    AppController(); 
    void begin();
    void tick();

    // 【新增】应用调度接口
    void startApp(AppBase* app); // 启动一个 App
    void quitApp();              // 退出当前 App

    // 【新增】延迟重载请求 (修复崩溃 bug)
    void scheduleReload() { reloadPending = true; }

    // 公开成员，方便 App 访问硬件 (或者通过 friend)
    MenuController menuCtrl; 
    StorageService storage;
    NetworkService network;

private:
    SystemToast toast;

    // --- 内部私有函数 ---
    void bindSystemEvents(); // 绑定系统默认按键  
    void destroyMenuTree();       
    void checkWeather();          
    void render();                
    void checkDayChange();       
    
    // --- 视图渲染器 ---
    PageWatchFace watchFace;       
    PageHorizontalMenu pageHorizontal; 
    PageVerticalMenu   pageVertical;   

    // --- 菜单数据 (Model) ---
    MenuPage* rootMenu = nullptr;
    MenuPage* toolsMenu = nullptr;

    // --- 状态标志 ---
    bool inMenuMode = false;      
    // 【新增】当前是否正在运行独立 App (游戏)
    AppBase* currentApp = nullptr; 

    // 【新增】重载标志位
    bool reloadPending = false;

    unsigned long timerWeather = 0;
    unsigned long timerSave = 0;

    // 后台任务相关
    TaskHandle_t weatherTaskHandle = NULL; 
    static void weatherTask(void* parameter); 

    // 页面垃圾回收站
    std::vector<MenuPage*> pageList; 

    MenuPage* createPage(const char* title, MenuLayout layout = LAYOUT_LIST) {
        MenuPage* p = new MenuPage(title, layout);
        pageList.push_back(p);
        return p;
    }
};