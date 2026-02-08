#pragma once

#include "model/AppBase.h"
#include "controller/AppController.h"

// -- 宏定义与常量 ----------------------------------------------------------------
#define ALARM_SUN (1 << 0)
#define ALARM_MON (1 << 1)
#define ALARM_TUE (1 << 2)
#define ALARM_WED (1 << 3)
#define ALARM_THU (1 << 4)
#define ALARM_FRI (1 << 5)
#define ALARM_SAT (1 << 6)

#define DEFAULT_WEEK_MASK 0x3E // 默认周一到周五

// -- 数据结构 --------------------------------------------------------------------
enum AlarmViewState {
    VIEW_LIST,  // 闹钟列表页
    VIEW_EDIT   // 闹钟编辑页
};

enum EditFocus {
    FOCUS_BACK_BTN = 0, // 顶部返回键
    FOCUS_SWITCH,       // 顶部开关
    FOCUS_TIME_HOUR,    // 中间时间(小时)
    FOCUS_TIME_MINUTE,  // 中间时间(分钟)
    FOCUS_WEEK_ROW,     // 底部的星期栏
    FOCUS_MAX           // 用于边界判断
};

// -- 类定义 ----------------------------------------------------------------------
class AlarmApp : public AppBase {
public:
    AlarmApp() {}
    virtual ~AlarmApp() {}

    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return false; }

private:
    AppController* sys = nullptr;
    AlarmViewState viewState = VIEW_LIST;
    EditFocus editFocus = FOCUS_TIME_HOUR;

    // 逻辑标志位
    bool isExiting = false;
    bool isEditingValue = false;
    bool isEditingWeekRow = false;

    // 动画与滚动变量
    float slideX = 0;
    int listIndex = -1; // -1=Back, 0~N=List
    float selectionSmooth = -1.0f;
    float scrollY = 0.0f;

    // 布局常量
    static constexpr int HEADER_H = 16;
    static constexpr int SPLIT_X = 88;
    static constexpr int RIGHT_CENTER = 108;
    static constexpr int LIST_ITEM_H = 16;

    // 交互变量
    int weekDayCursor = 1;
    unsigned long pressStartTime = 0;
    bool suppressClick = false;
    float deleteProgress = 0.0f;
    unsigned long lastFrameTime = 0;
    const float DELETE_SPEED = 1.5f;

    // 内部私有方法
    void saveAlarmData();
    void loadAlarmData();
    void onkeyUp();
    void onkeyDown();
    void onkeySelect();
    void handleLongPressLoop(float dt);
    int getAlarmCount();
    void initNewAlarm(int index);
    void sortAlarms();

    // 渲染方法
    void drawListView(int offsetX);
    void drawEditView(int offsetX);
    void render();
};