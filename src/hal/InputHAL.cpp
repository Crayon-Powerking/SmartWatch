#include "InputHAL.h"

InputHAL::InputHAL(uint8_t pin) : btn(pin, true, true) {
}

void InputHAL::begin() {
    btn.setPressMs(1000); // 长按阈值 1s

    btn.attachClick(_staticClickHandler, this);
    btn.attachLongPressStart(_staticLongPressHandler, this);
}

void InputHAL::tick() {
    btn.tick();
}

void InputHAL::attachClick(EventCallback cb) {
    _onClickCb = cb;
}

void InputHAL::attachLongPress(EventCallback cb) {
    _onLongPressCb = cb;
}

// --- 静态跳板函数的实现 ---
// 这里的 scope 就是 begin() 里传进去的 'this'
void InputHAL::_staticClickHandler(void* scope) {
    if (scope) {
        // 把 void* 还原成 InputHAL*，然后调用成员变量
        static_cast<InputHAL*>(scope)->_onClickCb();
    }
}

void InputHAL::_staticLongPressHandler(void* scope) {
    if (scope) {
        static_cast<InputHAL*>(scope)->_onLongPressCb();
    }
}