#pragma once

#include "model/AppBase.h"
#include "service/NetworkService.h"
#include <vector>

// -- 数据结构 --------------------------------------------------------------------

struct CitySlot {
    char name[16];   // 显示名称 (如 "Beijing")
    char code[32];   // API查询用的拼音 (如 "beijing")
    bool isEmpty;    // 是否为空
};

struct PresetCity {
    const char** names; // 多语言名称数组
    const char* code;   // 城市代码
};

enum ViewState {
    VIEW_MAIN,      // 0. 天气详情页
    VIEW_SLOTS,     // 1. 城市管理页 (仿 AlarmApp 列表)
    VIEW_LIBRARY    // 2. 预设库页
};

// -- 类定义 ----------------------------------------------------------------------
class WeatherApp : public AppBase {
public:
    WeatherApp() {}
    virtual ~WeatherApp() {}
    
    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return false; }

private:
    AppController* sys = nullptr;
    
    // 状态变量
    ViewState viewState = VIEW_MAIN;
    bool isExiting = false;

    // 数据模型
    CitySlot slots[5];                  // 5个固定槽位
    int activeSlotIndex = 0;            // 当前正在展示天气的城市
    int editingSlotIndex = -1;          // 当前正在编辑的槽位
    WeatherForecast forecast;           // 存储获取到的3天数据
    bool isLoading = false;             // 是否正在联网
    bool pendingWeatherUpdate = false;  // 标记需要刷新天气

    // UI 动画变量
    float selectionSmooth = 0.0f;
    float scrollY = 0.0f;               // 当前渲染位置
    int selectedIndex = 0;              // 当前选中的列表项索引 (-1 = Back)
    float slideX = 0.0f;                // 0=Main, -128=Slots, -256=Library
    float targetSlideX = 0.0f;

    // 布局常量 (完全复刻 AlarmApp)
    static constexpr int HEADER_H     = 16;
    static constexpr int SPLIT_X      = 88;
    static constexpr int RIGHT_CENTER = 108;
    static constexpr int LIST_ITEM_H  = 16;

    // 交互变量
    float deleteProgress = 0.0f;        // 删除进度 (0.0 - 1.0)
    unsigned long ignoreClickUntil = 0; // 防误触时间戳
    unsigned long lastFrameTime = 0;    // 上一帧时间戳

    // 预设城市列表
    static const std::vector<PresetCity> PRESETS;

    // 内部逻辑
    void loadSlots();
    void saveSlots();
    void refreshWeather();
    void deleteCurrentSlot();
    const char* getSlotName(int idx);
    const char* getWeatherText(int code);
    const uint8_t* getWeatherIcon(int code);

    // 按键处理
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();
    void onKeyLongPressSelect();
    void onKeyHoldSelect(float dt);

    // 渲染方法
    void render();
    void renderMainView();
    void renderSlotsView();
    void renderLibraryView();
};