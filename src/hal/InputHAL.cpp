#include "InputHAL.h"

InputHAL::InputHAL(uint8_t pin) : btn(pin, true, true) {
}

void InputHAL::begin() {
    // 这里的 300ms 对于长按来说有点短，通常建议 800ms 或 1000ms
    // 但如果你是为了快速测试连发，300ms 也是可以的
    btn.setPressMs(800); 

    // 【关键修改 1】现在可以放心地把这两行解除注释了
    // 因为下面的静态函数里加了“安全锁”，就算没绑定功能也不会崩
    btn.attachClick(_staticClickHandler, this);
    btn.attachLongPressStart(_staticLongPressHandler, this);
    btn.attachDuringLongPress(_staticDuringLongPressHandler, this);
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

void InputHAL::attachDuringLongPress(EventCallback cb) {
    _onDuringLongPressCb = cb;
}

// --- 静态跳板函数的实现 (核心修复区) ---

void InputHAL::_staticClickHandler(void* scope) {
    if (scope) {
        InputHAL* hal = static_cast<InputHAL*>(scope);
        // 【关键修改 2】必须先检查 _onClickCb 是否存在！
        // 如果 AppController 没绑定这个事件，直接调用会导致重启
        if (hal->_onClickCb) {
            hal->_onClickCb();
        }
    }
}

void InputHAL::_staticLongPressHandler(void* scope) {
    if (scope) {
        InputHAL* hal = static_cast<InputHAL*>(scope);
        // 【关键修改 3】加上判空保护
        // 之前重启就是因为这里没有 if，直接调用了空函数指针
        if (hal->_onLongPressCb) {
            hal->_onLongPressCb();
        }
    }
}

void InputHAL::_staticDuringLongPressHandler(void* scope) {
    if (scope) {
        InputHAL* hal = static_cast<InputHAL*>(scope);
        // 【关键修改 4】加上判空保护
        if (hal->_onDuringLongPressCb) {
            hal->_onDuringLongPressCb();
        }
    }
}