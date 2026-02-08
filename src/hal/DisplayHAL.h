#pragma once
#include <U8g2lib.h>
#include "AppConfig.h"
#include <cmath>

#define ORIGIN  0     // 起始坐标
#define MAX_X   128   // 最大横坐标
#define MAX_Y   64    // 最大纵坐标
#define LINE_Y  16    // 行高
#define WORD_Y  14    // 字高
#define GAP     2     // 间隙

class DisplayHAL {
public:
    DisplayHAL();                                                      // 构造函数
    void begin();                                                      // 初始化显示屏
    void clear();                                                      // 清空显示缓冲区
    void update();                                                     // 更新显示内容到屏幕

    // --- 字体与文本 ---
    void setFont(const uint8_t* font);                                  // 设置字体
    void setFontMode(uint8_t mode);                                     // 设置字体模式 (0=透明, 1=背景填充)
    int getStrWidth(const char* text);                                  // 获取字符串宽度
    void drawText(int x, int y, const char* text);                      // 画文本

    // --- 剪裁窗口 ---
    void setClipWindow(int x0, int y0, int x1, int y1);                 // 设置绘图限制区域 (左上角x0,y0, 右下角x1,y1)
    void setMaxClipWindow();                                            // 取消限制，恢复全屏绘制

    // --- 图形绘制 ---
    void drawIcon(int x, int y, int w, int h, const uint8_t* bitmap);   // 画图标 (XBM 位图)
    void drawGlyph(int x, int y, uint16_t encoding);                    // 画单个字符
    void drawFrame(int x, int y, int width, int height);                // 画空心矩形
    void drawRFrame(int x, int y, int width, int height,int radius);     // 画圆角空心矩形
    void drawBox(int x, int y, int width, int height);                  // 画实心矩形
    void drawRBox(int x, int y, int width, int height, int radius);     // 画圆角矩形
    void drawLine(int x1, int y1, int x2, int y2);                      // 画直线
    void setDrawColor(uint8_t color);                                   // 设置绘图颜色 (0=黑, 1=白, 2=反色)
    void drawCircle(int x0, int y0, int rad);                           // 画空心圆
    void drawDisc(int x0, int y0, int rad);                             // 画实心圆
    void drawPixel(int x, int y);                                       // 画单个像素
    void drawProgressArc(int x, int y, int radius, float progress);     // 画动态圆弧 (进度0.0~1.0)
    void setInvert(bool invert);                                        // 设置反色显示
    void setPowerSave(bool isEnable);                                   // 设置息屏模式
    void setAlwaysOn(bool isEnable);                                    // 设置常亮模式
    
private:
    U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;
};