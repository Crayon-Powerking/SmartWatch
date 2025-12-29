#include "InputHAL.h"

InputHAL::InputHAL() : btnBoot(PIN_BTN_BOOT, true, true) {
}

void InputHAL::begin() {
    btnBoot.setPressMs(1000); // 长按阈值 1s

    // --- 核心魔法区 ---
    // OneButton 支持 attachClick(函数, 上下文指针)
    // 我们把 'this' (当前对象的地址) 传进去
    btnBoot.attachClick(_staticClickHandler, this);
    btnBoot.attachLongPressStart(_staticLongPressHandler, this);
}

void InputHAL::tick() {
    btnBoot.tick();
}

void InputHAL::attachClick(EventCallback cb) {
    _onClickCb = cb;
}

void InputHAL::attachLongPress(EventCallback cb) {
    _onLongPressCb = cb;
}

// --- 静态跳板实现 ---

void InputHAL::_staticClickHandler(void* scope) {
    if (scope == nullptr) return;

    // 1. 把 void* 还原回 InputHAL* 指针
    InputHAL* self = static_cast<InputHAL*>(scope);

    // 2. 通过还原的指针，调用具体的成员变量
    if (self->_onClickCb) {
        self->_onClickCb();
    }
}

void InputHAL::_staticLongPressHandler(void* scope) {
    if (scope == nullptr) return;

    InputHAL* self = static_cast<InputHAL*>(scope);
    if (self->_onLongPressCb) {
        self->_onLongPressCb();
    }
}