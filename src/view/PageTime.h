#pragma once
#include "Page.h"
#include "service/NetworkService.h" // 引用服务

// 为了简单，我们这里暂时用外部指针
// 在 main.cpp 里我们会定义这个全局对象
extern NetworkService network;

class PageTime : public Page {
public:
    void draw(DisplayHAL* display) override {
        // 1. 获取时间字符串 "14:30"
        String timeStr = network.getTimeString();
        int sec = network.getSecond();

        // 2. 只有连网了才闪烁冒号
        if (network.isConnected()) {
             // 简单的闪烁逻辑: 奇数秒把冒号替换为空格
            if (sec % 2 != 0) {
                timeStr.replace(":", " ");
            }
            display->drawText(10, 20, "--- Time ---");
        } else {
            display->drawText(10, 20, "Connecting...");
        }

        // 3. 用大字体显示时间 (暂时还用 drawText，后面教你换字体)
        // String 转 char* 需要用 .c_str()
        display->drawText(20, 50, timeStr.c_str());
        
        // 底部显示一个小 IP 地址提示连接成功
        if (network.isConnected()) {
             display->drawText(5, 60, "WiFi OK");
        }
    }
};