#include "AboutApp.h"
#include "controller/AppController.h"
#include "assets/Lang.h"
#include "esp_system.h" 

extern InputHAL btnUp;
extern InputHAL btnDown;
extern InputHAL btnSelect;
extern DisplayHAL display;

void AboutApp::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    
    // 重置滚动状态
    currentScrollY = 0.0f;
    targetScrollY = 0;
    
    initInfo();

    // 绑定按键单击事件
    btnUp.attachClick(std::bind(&AboutApp::onKeyUp, this));
    btnDown.attachClick(std::bind(&AboutApp::onKeyDown, this));
    btnSelect.attachClick(std::bind(&AboutApp::onKeySelect, this));

    // 绑定按键长按事件
    btnUp.attachDuringLongPress(std::bind(&AboutApp::onKeyHoldUp, this));
    btnDown.attachDuringLongPress(std::bind(&AboutApp::onKeyHoldDown, this));
    btnSelect.attachLongPress(nullptr);
}

void AboutApp::onExit() {
    lines.clear();
}

void AboutApp::initInfo() {
    lines.clear();
    
    // --- 软件信息 ---
    lines.push_back("==== SOFTWARE ====");
    lines.push_back("SmartWatch OS");
    lines.push_back("Ver: v1.1.0");
    lines.push_back("By: Crayon");
    lines.push_back(""); 

    // --- 核心硬件 ---
    lines.push_back("====== CORE ======");
    lines.push_back("ESP32-WROOM");
    
    char buf[64];
    snprintf(buf, sizeof(buf), "Freq: %d MHz", ESP.getCpuFreqMHz());
    lines.push_back(buf);

    snprintf(buf, sizeof(buf), "Flash: %d MB", ESP.getFlashChipSize() / (1024 * 1024));
    lines.push_back(buf);

    uint32_t freeHeap = ESP.getFreeHeap();
    snprintf(buf, sizeof(buf), "Free RAM: %d KB", freeHeap/1024);
    lines.push_back(buf);
    
    lines.push_back("");

    // --- 外设 ---  
    lines.push_back("===== SENSORS ====");
    lines.push_back("Disp: SSD1306");

    lines.push_back("");
    lines.push_back("===== NETWORK ====");
    String mac = WiFi.macAddress();
    lines.push_back("MAC Address:");
    lines.push_back(mac); 

    lines.push_back("");

    totalContentHeight = lines.size() * lineHeight; 
}

int AboutApp::onLoop() {
    if (this->isExiting) return 1;

    display.clear();
    float diff = targetScrollY - currentScrollY;
    if (abs(diff) < 0.5) {
        currentScrollY = targetScrollY;
    } else {
        currentScrollY += diff * 0.25; 
    }

    draw();
    display.update();
    return 0; 
}

void AboutApp::draw() {
    display.setFont(u8g2_font_wqy12_t_gb2312); 
    display.setDrawColor(1);

    int screenH = 64;
    int drawY = (int)currentScrollY;
    
    // 1. 绘制文字
    for (int i = 0; i < lines.size(); i++) {
        int y = (i * lineHeight) - drawY + 12;

        if (y > -lineHeight && y < screenH + lineHeight) {
            if (lines[i].startsWith("=")) {
                display.drawBox(0, y - 9, 120, 11); 
                display.setDrawColor(0); 
                display.drawText(2, y, lines[i].c_str());
                display.setDrawColor(1); 
            } else {
                display.drawText(2, y, lines[i].c_str());
            }
        }
    }

    // 2. 绘制滚动条
    if (totalContentHeight > screenH) {
        int barX = 126;
        int barW = 2;
        
        int thumbH = (screenH * screenH) / totalContentHeight;
        if (thumbH < 6) thumbH = 6; 
        
        int maxScroll = totalContentHeight - screenH;
        int maxThumbY = screenH - thumbH;
        
        int thumbY = 0;
        if (maxScroll > 0) {
             thumbY = (drawY * maxThumbY) / maxScroll;
        }

        display.drawLine(barX + 1, 0, barX + 1, 64);
        display.drawBox(barX, thumbY, barW, thumbH);
    }
}

// 按键回调
void AboutApp::onKeyUp() {
    targetScrollY -= 28; 
    if (targetScrollY < 0) targetScrollY = 0;
}

void AboutApp::onKeyDown() {
    int maxScroll = totalContentHeight - 64;
    targetScrollY += 28;
    if (targetScrollY > maxScroll) targetScrollY = maxScroll;
}

void AboutApp::onKeyHoldUp() {
    targetScrollY -= 1; 
    if (targetScrollY < 0) targetScrollY = 0;
}

void AboutApp::onKeyHoldDown() {
    int maxScroll = totalContentHeight - 64;
    targetScrollY += 1;
    if (targetScrollY > maxScroll) targetScrollY = maxScroll;
}

void AboutApp::onKeySelect() {
    this->isExiting = true;
}