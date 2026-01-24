#include "hal/DisplayHAL.h"

DisplayHAL::DisplayHAL()
    : u8g2(U8G2_R0, PIN_OLED_CS, PIN_OLED_DC, PIN_OLED_RES) {
}

void DisplayHAL::begin() {
    u8g2.begin();
    u8g2.setFont(u8g2_font_ncenB10_tr); 
    u8g2.enableUTF8Print();
    u8g2.setFontMode(1); 
}

void DisplayHAL::clear() {
    u8g2.clearBuffer();
}

void DisplayHAL::update() {
    u8g2.sendBuffer();
}

void DisplayHAL::setClipWindow(int x0, int y0, int x1, int y1) {
    u8g2.setClipWindow(x0, y0, x1, y1);
}

void DisplayHAL::setMaxClipWindow() {
    u8g2.setMaxClipWindow();
}

void DisplayHAL::setFont(const uint8_t* font) {
    u8g2.setFont(font);
}

void DisplayHAL::setFontMode(uint8_t mode) {
    u8g2.setFontMode(mode);
}

int DisplayHAL::getStrWidth(const char* text) {
    return u8g2.getUTF8Width(text);
}

void DisplayHAL::drawText(int x, int y, const char* text) {
    u8g2.drawUTF8(x, y, text);
}

void DisplayHAL::drawGlyph(int x, int y, uint16_t encoding) {
    u8g2.drawGlyph(x, y, encoding);
}

void DisplayHAL::drawFrame(int x, int y, int width, int height) {
    u8g2.drawFrame(x, y, width, height);
}

void DisplayHAL::drawBox(int x, int y, int width, int height) {
    u8g2.drawBox(x, y, width, height);
}

void DisplayHAL::drawRBox(int x, int y, int width, int height, int radius) {
    u8g2.drawRBox(x, y, width, height, radius);
}

void DisplayHAL::drawLine(int x1, int y1, int x2, int y2) {
    u8g2.drawLine(x1, y1, x2, y2);
}

void DisplayHAL::drawIcon(int x, int y, int w, int h, const uint8_t* bitmap) {
    u8g2.drawXBMP(x, y, w, h, bitmap);
}

void DisplayHAL::setDrawColor(uint8_t color) {
    u8g2.setDrawColor(color);
}

void DisplayHAL::drawCircle(int x0, int y0, int rad) {
    u8g2.drawCircle(x0, y0, rad, U8G2_DRAW_ALL);
}

void DisplayHAL::drawDisc(int x0, int y0, int rad) {
    u8g2.drawDisc(x0, y0, rad, U8G2_DRAW_ALL);
}

// 核心实现：画动态圆弧
// 原理：将圆分割成小线段，根据进度画出对应的线段
void DisplayHAL::drawProgressArc(int x, int y, int radius, float progress) {
    if (progress <= 0.0f) return;
    if (progress > 1.0f) progress = 1.0f;

    // 起始角度：-PI/2 是12点钟方向
    float startAngle = -M_PI / 2.0f; 
    // 结束角度：一圈是 2*PI
    float endAngle = startAngle + (progress * 2.0f * M_PI);

    // 步进角度：为了画圆滑，每10度或15度画一根线
    // 步进越小越圆滑，但计算量越大。0.2弧度(约11度)对OLED足够了
    float step = 0.2f; 

    float currentAngle = startAngle;
    
    // 计算起点的坐标
    int prevX = x + (int)(cos(currentAngle) * radius);
    int prevY = y + (int)(sin(currentAngle) * radius);

    while (currentAngle < endAngle) {
        currentAngle += step;
        if (currentAngle > endAngle) currentAngle = endAngle; // 修正最后一步

        int currX = x + (int)(cos(currentAngle) * radius);
        int currY = y + (int)(sin(currentAngle) * radius);

        u8g2.drawLine(prevX, prevY, currX, currY);

        prevX = currX;
        prevY = currY;
    }
}