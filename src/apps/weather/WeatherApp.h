#pragma once

#include "model/AppBase.h"
#include "service/NetworkService.h"
#include <vector>

// 定义一个城市槽位
struct CitySlot {
    char name[16];   // 显示名称 (如 "Beijing")
    char code[32];   // API查询用的拼音 (如 "beijing")
    bool isEmpty;    // 是否为空
};

// 定义预设城市 (用于第三层列表)
struct PresetCity {
    const char* name;
    const char* code;
};

class WeatherApp : public AppBase {
public:
    WeatherApp(AppController* sys) : sys(sys) {}
    
    // --- AppBase 生命周期接口 ---
    void onRun(AppController* sys) override;
    int  onLoop() override;
    void onExit() override;

private:
    AppController* sys;

    // --- 状态定义 ---
    enum ViewState {
        VIEW_MAIN,      // 0. 天气详情页 (显示3天预报)
        VIEW_SLOTS,     // 1. 槽位管理页 (选择或删除)
        VIEW_LIBRARY    // 2. 预设库页 (从列表添加)
    };

    ViewState viewState = VIEW_MAIN;

    // --- 数据模型 ---
    CitySlot slots[5];          // 5个固定槽位
    int activeSlotIndex = 0;    // 【职责A】当前正在展示天气的城市（不要随便动它）
    int editingSlotIndex = -1;// 当前正在看哪个槽位
    WeatherForecast forecast;   // 存储获取到的3天数据
    bool isLoading = false;     // 是否正在联网
    // [新增] 延迟刷新标志位
    bool pendingWeatherUpdate = false;

    // --- 预设城市列表 (硬编码数据) ---
    // 我们将在 .cpp 里初始化这个列表
    static const std::vector<PresetCity> PRESETS;

    // 新增：用于光标平滑动画的浮点数
    float selectionSmooth = 0.0f;
    // --- UI 动画变量 (果冻效果核心) ---
    float scrollY = 0;      // 当前渲染位置
    float targetY = 0;      // 目标位置
    int   selectedIndex = 0;// 当前选中的列表项索引

    // 视图切换动画 (X轴滑动)
    float slideX = 0;       // 0=Main, -128=Slots, -256=Library
    float targetSlideX = 0;

    bool isDeleting = false;        // 是否正在按住删除
    float deleteProgress = 0.0f;    // 删除进度 (0.0 - 1.0)
    unsigned long pressStartTime = 0; // 按下开始的时间
    unsigned long lastHoldTick = 0; // 用于检测长按是否中断

    // [新增] 防误触计时器
    unsigned long ignoreClickUntil = 0; 
    
    // [新增] 用于计算时间差
    unsigned long lastFrameTime = 0;

    // --- 内部逻辑函数 ---
    void handleInput();         // 按键处理
    void render();              // 总渲染入口
    
    // 子渲染函数
    void renderMainView();      // 画天气
    void renderSlotsView();     // 画槽位
    void renderLibraryView();   // 画预设库

    const char* getWeatherText(int code);
    const uint8_t* getWeatherIcon(int code);
    void loadSlots();           // 从 Storage 读取槽位
    void saveSlots();           // 保存槽位到 Storage
    void refreshWeather();      // 联网动作

    void deleteCurrentSlot();   // 执行删除动作
    void drawTrashIcon(int x, int y); // 画垃圾桶
    int getCityCount(); // 统计非空城市数量
};