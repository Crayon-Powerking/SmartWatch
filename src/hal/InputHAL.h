#pragma once
#include <Arduino.h>
#include <OneButton.h>
#include <functional>
#include "AppConfig.h"

class InputHAL {
public:
    using EventCallback = std::function<void()>;        // 定义一个通用的回调函数类型：不接受参数，返回 void
    InputHAL(uint8_t pin);                              // 构造函数：需要传入引脚号
    void begin();                                       // 初始化按键
    void tick();                                        // 按键轮询心跳
    bool isPressed();                                   // 检查按键当前是否被按下

    void attachClick(EventCallback cb);                 // 注册单击事件回调
    void attachLongPress(EventCallback cb);             // 注册长按开始事件回调
    void attachDuringLongPress(EventCallback cb);       // 注册长按持续事件回调

private:
    OneButton btn;
    uint8_t _pin;
    // --- 内部存储的回调 ---
    EventCallback _onClickCb = nullptr;
    EventCallback _onLongPressCb = nullptr;
    EventCallback _onDuringLongPressCb = nullptr;

    // --- 静态跳板函数 (Static Wrappers) ---
    // 只有静态成员函数才能匹配 C 语言库的函数指针要求
    // void* scope 是 OneButton 提供的“上下文”，我们会把 'this' 指针传进去
    static void _staticClickHandler(void* scope);
    static void _staticLongPressHandler(void* scope);
    static void _staticDuringLongPressHandler(void* scope);
};