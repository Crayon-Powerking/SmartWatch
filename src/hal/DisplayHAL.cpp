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

// --- 新增的实现 ---
void DisplayHAL::setClipWindow(int x0, int y0, int x1, int y1) {
    u8g2.setClipWindow(x0, y0, x1, y1);
}

void DisplayHAL::setMaxClipWindow() {
    u8g2.setMaxClipWindow();
}
// ------------------

void DisplayHAL::setFont(const uint8_t* font) {
    u8g2.setFont(font);
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

void DisplayHAL::drawLine(int x1, int y1, int x2, int y2) {
    u8g2.drawLine(x1, y1, x2, y2);
}

void DisplayHAL::drawIcon(int x, int y, int w, int h, const uint8_t* bitmap) {
    u8g2.drawXBMP(x, y, w, h, bitmap);
}

void DisplayHAL::setDrawColor(uint8_t color) {
    u8g2.setDrawColor(color);
}