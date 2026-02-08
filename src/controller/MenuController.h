#pragma once

#include <vector>
#include <Arduino.h>
#include "model/MenuTypes.h"

// -- 类定义 ----------------------------------------------------------------------
class MenuController {
public:
    void init(MenuPage* root) {
        rootMenu = root;
        menuStack.clear();
        current = rootMenu;
        if (current) visualIndex = (float)current->selectedIndex;
    }

    // -- 动画核心：Lerp 插值 -----------------------------------------------------
    void tick() {
        if (!current) return;
        float target = (float)current->selectedIndex;
        float diff = target - visualIndex;

        if (abs(diff) < 0.01) {
            visualIndex = target;
        } else {
            visualIndex += diff * 0.15; // 0.15 为动画速度系数
        }
    }

    // -- 导航控制 ----------------------------------------------------------------
    void next() {
        if (current && current->selectedIndex < (int)current->items.size() - 1) current->selectedIndex++;
    }

    void prev() {
        if (current && current->selectedIndex > 0) current->selectedIndex--;
    }

    void enter() {
        if (current && !current->items.empty()) {
            MenuItem& item = current->items[current->selectedIndex];
            if (item.action) item.action();
        }
    }

    void enter(MenuPage* subPage) {
        if (subPage && current != subPage) {
            menuStack.push_back(current);
            current = subPage;
            visualIndex = (float)current->selectedIndex;
        }
    }

    bool back() {
        if (menuStack.empty()) return false;
        current = menuStack.back();
        menuStack.pop_back();
        visualIndex = (float)current->selectedIndex;
        return true;
    }

    // -- 获取器 ------------------------------------------------------------------
    float getVisualIndex() { return visualIndex; }
    MenuPage* getCurrentPage() { return current; }

private:
    MenuPage* rootMenu = nullptr;
    MenuPage* current = nullptr;
    std::vector<MenuPage*> menuStack;
    float visualIndex = 0.0;
};