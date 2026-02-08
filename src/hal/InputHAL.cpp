#include "InputHAL.h"

// -- 构造与初始化 ----------------------------------------------------------------

InputHAL::InputHAL(uint8_t pin) 
    : btn(pin, true, true), _pin(pin) {
}

void InputHAL::begin() {
    btn.setPressMs(800);                                             // 长按时间设为 800ms
    btn.attachClick(_staticClickHandler, this);                      // 绑定单击事件
    btn.attachLongPressStart(_staticLongPressHandler, this);         // 绑定长按开始事件        
    btn.attachDuringLongPress(_staticDuringLongPressHandler, this);  // 绑定长按持续事件
}

// -- 状态轮询 --------------------------------------------------------------------

void InputHAL::tick() {
    btn.tick();
}

bool InputHAL::isPressed() {
    return digitalRead(_pin) == LOW;
}

// -- 事件绑定 --------------------------------------------------------------------

void InputHAL::attachClick(EventCallback cb) {
    _onClickCb = cb;
}

void InputHAL::attachLongPress(EventCallback cb) {
    _onLongPressCb = cb;
}

void InputHAL::attachDuringLongPress(EventCallback cb) {
    _onDuringLongPressCb = cb;
}

// -- 静态跳板函数实现 ------------------------------------------------------------

void InputHAL::_staticClickHandler(void* scope) {
    if (scope) {
        InputHAL* hal = static_cast<InputHAL*>(scope);               // 还原为对象指针
        if (hal->_onClickCb) {
            hal->_onClickCb();
        }
    }
}

void InputHAL::_staticLongPressHandler(void* scope) {
    if (scope) {
        InputHAL* hal = static_cast<InputHAL*>(scope);
        if (hal->_onLongPressCb) {
            hal->_onLongPressCb();
        }
    }
}

void InputHAL::_staticDuringLongPressHandler(void* scope) {
    if (scope) {
        InputHAL* hal = static_cast<InputHAL*>(scope);
        if (hal->_onDuringLongPressCb) {
            hal->_onDuringLongPressCb();
        }
    }
}