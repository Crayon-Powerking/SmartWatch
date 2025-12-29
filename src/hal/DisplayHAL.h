#pragma once

#include <U8g2lib.h>
#include "AppConfig.h"

/**
 * @brief 屏幕硬件抽象层 (Hardware Abstraction Layer)
 * @note  封装 U8g2 库，向业务层提供统一的绘图接口
 */
class DisplayHAL {
public:
    // 构造函数
    DisplayHAL();

    // 初始化屏幕 (上电后必须调用)
    void begin();

    // --- 显存操作 ---
    void clear();   // 清空缓冲区
    void update();  // 将缓冲区内容发送到物理屏幕 (U8g2 buffer -> Screen)

    // --- 绘图原语 (根据未来需求不断扩展) ---
    void drawText(int x, int y, const char* text);
    // 未来可扩展: drawIcon(), drawLine(), setBrightness()...

private:
    // 组合优于继承：我们将 U8g2 对象作为私有成员藏起来
    // 使用 F_4W_HW_SPI 模式：全缓冲区(F) + 4线硬件SPI
    U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;
};