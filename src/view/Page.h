#pragma once

#include "hal/DisplayHAL.h"
#include "model/AppData.h"

// -- 按键定义 ----------------------------------------------------------------------
#define EVENT_KEY_UP    1
#define EVENT_KEY_DOWN  2
#define EVENT_KEY_ENTER 3

// -- 类定义 ----------------------------------------------------------------------
class Page {
public:
    virtual ~Page() {}                              // 虚析构函数   
    virtual void draw(DisplayHAL* display) = 0;     // 画面内容绘制
    virtual void onButton(int id) {}                // 按键事件处理
};