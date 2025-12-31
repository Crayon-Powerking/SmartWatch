#pragma once
#include "Page.h"
#include "PageWatchFace.h" 
// #include "PageSteps.h"   <-- 删掉
// #include "PageBattery.h" <-- 删掉
// #include "PageAbout.h"   <-- 删掉
#include <vector>

class PageManager {
public:
    PageManager() {
        // 现在只加载这一个核心页面
        pages.push_back(new PageWatchFace());    
        
        // 删掉其他的 push_back
        
        currentPageIndex = 0; 
    }

    // 析构函数：养成好习惯，new 出来的东西要 delete
    // 虽然在嵌入式里 main 循环永远不结束，但写上是好习惯
    ~PageManager() {
        for (auto p : pages) delete p;
        pages.clear();
    }

    Page* getCurrentPage() {
        // 安全检查：防止万一没有页面报错
        if (pages.empty()) return nullptr;
        return pages[currentPageIndex];
    }

    // 暂时让切换功能失效，或者循环自己
    void switchNext() {
        // 目前只有一页，切不切都一样，留着这个空壳函数
        // 是为了不让 main.cpp 报错
    }

    void render(DisplayHAL* display) {
        if (getCurrentPage()) {
            display->clear();
            getCurrentPage()->draw(display);
            display->update();
        }
    }

    void dispatchInput(int keyId) {
        if (getCurrentPage()) {
            getCurrentPage()->onButton(keyId);
        }
    }

private:
    std::vector<Page*> pages; 
    int currentPageIndex = 0;
};