#pragma once
#include "hal/DisplayHAL.h"
#include "model/AppData.h"

// 定义按键事件的代号 (方便页面识别)
#define EVENT_KEY_UP    1
#define EVENT_KEY_DOWN  2
#define EVENT_KEY_ENTER 3 // 对应 BOOT/Select 键

// [C++ 核心概念]: 类 (Class)
class Page {
public:
    // [语法解释]: virtual ~Page() {}
    // 虚析构函数：这是安全带。保证页面被销毁时，清理工作能做得干干净净。
    virtual ~Page() {}

    // [语法解释]: virtual void draw(...) = 0;
    // 纯虚函数 (Pure Virtual Function)
    // 含义：这是“家规”。父类说：“我不知道具体怎么画，但你们这些子类必须实现这个函数，否则不准出生！”
    // 参数：传入 DisplayHAL 指针，让页面拿着画笔去画。
    virtual void draw(DisplayHAL* display) = 0;

    // [语法解释]: virtual void onButton(...) {}
    // 虚函数 (非纯虚)
    // 含义：父类提供了一个默认的“空操作”。如果子类想处理按键，就重写它；不想处理，就用我这个默认的（啥都不干）。
    virtual void onButton(int id) {}
};