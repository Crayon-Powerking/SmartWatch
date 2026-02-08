#include "AboutApp.h"
#include "controller/AppController.h"
#include "assets/Lang.h"
#include "esp_system.h" 
#include <cmath>

// -- 辅助函数 --------------------------------------------------------------------

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

    snprintf(buf, sizeof(buf), "Flash: %u MB", ESP.getFlashChipSize() / (1024 * 1024));
    lines.push_back(buf);

    uint32_t freeHeap = ESP.getFreeHeap();
    snprintf(buf, sizeof(buf), "Free RAM: %u KB", freeHeap / 1024);
    lines.push_back(buf);
    
    lines.push_back("");

    // --- 外设 ---  
    lines.push_back("===== SENSORS ====");
    lines.push_back("Disp: SSD1306");
    lines.push_back("Imu: MPU6050");

    lines.push_back("");
    lines.push_back("===== NETWORK ====");
    String mac = WiFi.macAddress();
    lines.push_back("MAC Address:");
    lines.push_back(mac); 

    lines.push_back("");

    totalContentHeight = lines.size() * lineHeight; 
}

// -- 按键处理 --------------------------------------------------------------------

void AboutApp::onKeyUp() {
    targetScrollY -= 28.0f;
    if (targetScrollY < 0) targetScrollY = 0;
}

void AboutApp::onKeyDown() {
    int maxScroll = totalContentHeight - 64;
    if (maxScroll < 0) maxScroll = 0;

    targetScrollY += 28.0f;
    if (targetScrollY > maxScroll) targetScrollY = (float)maxScroll;
}

void AboutApp::onKeySelect() {
    this->isExiting = true;
}

void AboutApp::onKeyHoldUp() {
    // 长按时平滑连续滚动
    targetScrollY -= 2.0f;
    if (targetScrollY < 0) targetScrollY = 0;
}

void AboutApp::onKeyHoldDown() {
    int maxScroll = totalContentHeight - 64;
    if (maxScroll < 0) maxScroll = 0;

    targetScrollY += 2.0f;
    if (targetScrollY > maxScroll) targetScrollY = (float)maxScroll;
}

// -- 生命周期 --------------------------------------------------------------------

void AboutApp::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    
    // 重置滚动状态
    currentScrollY = 0.0f;
    targetScrollY = 0.0f;
    
    initInfo();

    // 绑定按键单击事件
    sys->btnUp.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeyUp();
    });

    sys->btnDown.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeyDown();
    });

    sys->btnSelect.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeySelect();
    });

    // 绑定按键长按事件
    sys->btnUp.attachDuringLongPress([this]() {
        this->onKeyHoldUp();
    });

    sys->btnDown.attachDuringLongPress([this]() {
        this->onKeyHoldDown();
    });

    sys->btnSelect.attachLongPress(nullptr);
}

int AboutApp::onLoop() {
    if (this->isExiting) return 1;

    sys->display.clear();

    // 滚动阻尼动画
    float diff = targetScrollY - currentScrollY;
    if (fabs(diff) < 0.5f) {
        currentScrollY = targetScrollY;
    } else {
        currentScrollY += diff * 0.25f; 
    }

    draw();
    return 0; 
}

void AboutApp::onExit() {
    lines.clear();
}

// -- 绘图渲染 --------------------------------------------------------------------

void AboutApp::draw() {
    sys->display.setFont(u8g2_font_wqy12_t_gb2312); 
    sys->display.setDrawColor(1);

    int screenH = 64;
    int drawY = (int)currentScrollY;
    
    // 1. 绘制文字
    for (size_t i = 0; i < lines.size(); i++) {
        int y = (i * lineHeight) - drawY + 12;

        // 仅绘制屏幕范围内的文字
        if (y > -lineHeight && y < screenH + lineHeight) {
            if (lines[i].startsWith("=")) {
                // 标题栏反色显示
                sys->display.drawBox(0, y - 9, 120, 11); 
                sys->display.setDrawColor(0); 
                sys->display.drawText(2, y, lines[i].c_str());
                sys->display.setDrawColor(1); 
            } else {
                sys->display.drawText(2, y, lines[i].c_str());
            }
        }
    }

    // 2. 绘制滚动条
    if (totalContentHeight > screenH) {
        int barX = 126;
        int barW = 2;
        
        // 计算滑块高度
        int thumbH = (screenH * screenH) / totalContentHeight;
        if (thumbH < 6) thumbH = 6; 
        
        int maxScroll = totalContentHeight - screenH;
        int maxThumbY = screenH - thumbH;
        
        int thumbY = 0;
        if (maxScroll > 0) {
             thumbY = (drawY * maxThumbY) / maxScroll;
        }

        sys->display.drawLine(barX + 1, 0, barX + 1, 64);
        sys->display.drawBox(barX, thumbY, barW, thumbH);
    }
}