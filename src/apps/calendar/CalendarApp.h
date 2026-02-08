#pragma once

#include "model/AppBase.h"
#include "service/NetworkService.h"
#include <ctime>

// -- 数据结构 --------------------------------------------------------------------
struct CalendarCache {
    HolidayInfo holiday;    // 节日数据
    int fortuneIndex;       // 运势索引
    int dayStamp;           // 记录上次存储的日期 (如 20260208)
    bool isRevealed;        // 今日是否已揭晓运势
};

// -- 类定义 ----------------------------------------------------------------------
class CalendarApp : public AppBase {
public:
    CalendarApp() {}
    virtual ~CalendarApp() {}

    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;

private:
    AppController* sys = nullptr;
    bool isExiting = false;

    // 数据
    CalendarCache cache;
    bool needNetworkUpdate = false;

    // 交互与动画
    float holdProgress = 0.0f;
    unsigned long lastFrameTime = 0;

    // 内部逻辑
    void loadCache();
    void saveCache();
    void checkDayChange();
    void generateDailyFortune();

    // 按键处理
    void onKeySelect();
    void onKeyUp();

    // 渲染方法
    void render();
    void renderHolidayInfo();
    void renderFortuneReveal();
};