#pragma once

#include <Arduino.h>
#include <OneButton.h>
#include <functional> // 必须引用，用于 std::function
#include "AppConfig.h"

class InputHAL {
public:
    // 定义一个通用的回调函数类型：不接受参数，返回 void
    using EventCallback = std::function<void()>;

    // 构造函数：需要传入引脚号
    InputHAL(uint8_t pin); 

    void begin();
    void tick();

    // 注册接口：业务层把函数传进来
    void attachClick(EventCallback cb);
    void attachLongPress(EventCallback cb);

private:
    OneButton btn;

    // --- 内部存储的回调 ---
    EventCallback _onClickCb = nullptr;
    EventCallback _onLongPressCb = nullptr;

    // --- 静态跳板函数 (Static Wrappers) ---
    // 只有静态成员函数才能匹配 C 语言库的函数指针要求
    // void* scope 是 OneButton 提供的“上下文”，我们会把 'this' 指针传进去
    static void _staticClickHandler(void* scope);
    static void _staticLongPressHandler(void* scope);
};