#include "DisplayHAL.h"

DisplayHAL::DisplayHAL() 
    : u8g2(U8G2_R0, PIN_OLED_CS, PIN_OLED_DC, PIN_OLED_RES) {
}

void DisplayHAL::begin() {
    u8g2.begin();
    
    // 设置一个比较美观的无衬线粗体 (10px高度)
    // _tr 后缀表示: Transparent (背景透明)
    u8g2.setFont(u8g2_font_ncenB10_tr); 
    
    // 开启 UTF8 支持 (如果未来要显示简单中文)
    u8g2.enableUTF8Print();
}

void DisplayHAL::clear() {
    u8g2.clearBuffer();
}

void DisplayHAL::update() {
    u8g2.sendBuffer();
}

void DisplayHAL::drawText(int x, int y, const char* text) {
    u8g2.drawStr(x, y, text);
}