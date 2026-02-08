#pragma once

#include "model/AppBase.h"
#include "controller/AppController.h"

// -- 宏定义与常量 ----------------------------------------------------------------
#define SENSITIVITY       1.5f  // 灵敏度：数值越大，小球移动越快
#define MAX_VISIBLE_ANGLE 20    // 超过20度小球就碰壁
#define CENTER_X          64    // 屏幕中心 X
#define CENTER_Y          31    // 屏幕中心 Y
#define BIG_RADIUS        31    // 大圆半径
#define BALL_RADIUS       4     // 小球半径

// -- 类定义 ----------------------------------------------------------------------
class Gyroscope : public AppBase {
public:
    Gyroscope() {}
    virtual ~Gyroscope() {}

    void onRun(AppController* sys) override;
    int onLoop() override;
    void onExit() override;
    virtual bool isKeepAlive() override { return true; }

private:
    AppController* sys = nullptr;

    // 状态变量
    bool isGraphMode = true;
    bool isExiting = false;

    // 按键处理
    void onKeyUp();
    void onKeyDown();
    void onKeySelect();

    // 渲染方法
    void render();
    void drawGraphicMode();
    void drawDataMode();
};