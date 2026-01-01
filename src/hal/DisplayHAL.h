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
    void setFont(const uint8_t* font);
    int getStrWidth(const char* text);
    void drawText(int x, int y, const char* text);

    // --- 剪裁窗口 (新增) ---
    // 设置绘图限制区域 (左上角x0,y0, 右下角x1,y1)
    void setClipWindow(int x0, int y0, int x1, int y1);
    // 取消限制，恢复全屏绘制
    void setMaxClipWindow();

    // --- 图形绘制 ---
    void drawIcon(int x, int y, int w, int h, const uint8_t* bitmap);
    void drawGlyph(int x, int y, uint16_t encoding);
    void drawFrame(int x, int y, int width, int height);
    void drawBox(int x, int y, int width, int height);
    void drawLine(int x1, int y1, int x2, int y2);

private:
    U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;
};