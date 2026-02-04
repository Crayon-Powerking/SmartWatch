#pragma once
#include <vector>
#include "AppConfig.h"
#include "model/AppData.h"
#include "model/MenuTypes.h"
#include "model/AppBase.h"
#include "hal/DisplayHAL.h"
#include "hal/InputHAL.h"
#include "hal/ImuHAL.h"
#include "service/NetworkService.h"
#include "service/StorageService.h"
#include "view/PageWatchFace.h"
#include "view/PageHorizontalMenu.h" 
#include "view/PageVerticalMenu.h"
#include "view/SystemToast.h"
#include "controller/MenuController.h"

class MenuFactory;
class SettingsBuilder;
class GamesBuilder;
class ToolBuilder;

class AppController {
    
    friend class MenuFactory;
    friend class SettingsBuilder;
    friend class ToolBuilder;
    friend class GamesBuilder;
    friend class AppBase; 

public:
    AppController(); 
    void begin();
    void tick();
    void forceWeatherUpdate();
    void startApp(AppBase* app);                     // 启动一个 App
    void quitApp();                                  // 退出当前 App
    void scheduleReload() { reloadPending = true; }  // 安排重载菜单树
    void onButtonEvent();                            // 喂狗

    // 公开成员，方便 App 访问硬件 (或者通过 friend)
    MenuController menuCtrl; 
    StorageService storage;
    NetworkService network;

    DisplayHAL display;
    InputHAL btnSelect;
    InputHAL btnUp;
    InputHAL btnDown;
    ImuHAL imu;

    unsigned long getLastActiveTime() const { 
        return lastActiveTime; 
    }
private:
    SystemToast toast;

    // --- 内部私有函数 ---
    void bindSystemEvents();            // 绑定系统默认按键  
    void destroyMenuTree();             // 销毁菜单树     
    void checkWeather();                // 检查并更新天气数据     
    void render();                      // 渲染当前画面   
    void checkDayChange();              // 检查是否跨天，重置步数  

    // --- 睡眠管理 ---
    void checkSleep();                  // 检查是否该睡觉了
    void wakeUp();                      // 唤醒屏幕
    bool isSleeping = false;            // 当前是否黑屏
    unsigned long lastActiveTime = 0;   // 上次操作时间
    
    // --- 视图渲染器 ---
    PageWatchFace      watchFace;       // 表盘页面
    PageHorizontalMenu pageHorizontal;  // 横向菜单页面
    PageVerticalMenu   pageVertical;    // 纵向菜单页面

    // --- 菜单数据 (Model) ---
    MenuPage* rootMenu = nullptr;       // 根菜单
    MenuPage* toolsMenu = nullptr;      // 工具子菜单

    // --- 状态标志 ---
    bool inMenuMode = false;            // 当前是否在菜单模式 (否则为表盘模式)
    AppBase* currentApp = nullptr;      // 当前运行的 App 实例
    bool reloadPending = false;         // 是否有菜单重载请求待处理
    unsigned long timerWeather = 0;     // 天气更新时间戳
    unsigned long timerSave = 0;        // 上次保存数据时间戳

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