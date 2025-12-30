#pragma once
#include "Page.h"
#include "PageSteps.h"
#include "PageBattery.h"
#include "PageAbout.h"
#include "PageWatchFace.h"
#include <vector> // C++ 的动态数组

class PageManager {
public:
    PageManager() {
        // 实例化所有的页面，放进列表里
        // new PageSteps() 会在堆内存创建对象，返回一个指针
        pages.push_back(new PageWatchFace());    // 0: UI
        pages.push_back(new PageSteps());   // 1: 步数
        pages.push_back(new PageBattery()); // 2: 电量
        pages.push_back(new PageAbout());  // 3: 关于
        
        currentPageIndex = 0; // 默认看第 0 页
    }

    // 获取当前页面
    Page* getCurrentPage() {
        return pages[currentPageIndex];
    }

    // 切换到下一页
    void switchNext() {
        currentPageIndex++;
        if (currentPageIndex >= pages.size()) {
            currentPageIndex = 0; // 循环回到开头
        }
    }

    // --- 核心：多态绘制 ---
    // Main 只需要调用这个，不用管现在是哪个页面
    void render(DisplayHAL* display) {
        display->clear();
        
        getCurrentPage()->draw(display);
        
        display->update();
    }

    // --- 核心：事件分发 ---
    // 将按键事件转发给当前页面
    void dispatchInput(int keyId) {
        getCurrentPage()->onButton(keyId);
    }

private:
    // std::vector 是 C++ 的数组，比 C 数组好用在它知道自己有多长
    std::vector<Page*> pages; 
    int currentPageIndex = 0;
};