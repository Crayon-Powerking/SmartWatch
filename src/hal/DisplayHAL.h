#pragma once
#include <U8g2lib.h>
#include "AppConfig.h"

class DisplayHAL {
public:
    DisplayHAL();
    void begin();
    void clear();
    void update();

    // --- 字体与文本 ---
    // 允许传入 u8g2 的字体数据 (const uint8_t*)
    void setFont(const uint8_t* font);
    // 获取文字宽度 (用于居中计算)
    int getStrWidth(const char* text);
    void drawText(int x, int y, const char* text);

    // --- 图形绘制 ---

    // 画单个图标位图 (比如脚印图标)
    void drawIcon(int x, int y, int w, int h, const uint8_t* bitmap);
    
    // 画单个图标字符 (比如 Open Iconic 里的 ID)
    void drawGlyph(int x, int y, uint16_t encoding);
    
    // 画空心框 (电池外壳)
    void drawFrame(int x, int y, int width, int height);
    // 画实心块 (电池电量)
    void drawBox(int x, int y, int width, int height);
    // 画线 (分割线)
    void drawLine(int x1, int y1, int x2, int y2);

private:
    U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;
};