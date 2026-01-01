#pragma once
#include <vector>
#include <Arduino.h> // 需要 abs()
#include "model/MenuTypes.h"

class MenuController {
public:
    void init(MenuPage* root) {
        rootMenu = root;
        menuStack.clear();
        current = rootMenu;
        // 初始化时，视觉位置直接等于逻辑位置，避免开机动画乱飞
        if (current) visualIndex = (float)current->selectedIndex;
    }

    // --- 动画核心：每一帧都计算插值 ---
    void tick() {
        if (!current) return;
        
        float target = (float)current->selectedIndex;
        float diff = target - visualIndex;

        // 简单的 Lerp (线性插值) 算法
        // 0.2 是速度系数 (0.1慢 ~ 0.5快)
        // 如果差距很小，直接吸附，避免浮点数抖动
        if (abs(diff) < 0.01) {
            visualIndex = target;
        } else {
            visualIndex += diff * 0.15;
        }
    }

    // 获取用于绘图的“动画坐标”
    float getVisualIndex() {
        return visualIndex;
    }

    void next() {
        if (!current) return;
        if (current->selectedIndex < current->items.size() - 1) {
            current->selectedIndex++;
        }
    }

    void prev() {
        if (!current) return;
        if (current->selectedIndex > 0) {
            current->selectedIndex--;
        }
    }

    void enter() {
        if (!current || current->items.empty()) return;
        MenuItem& item = current->items[current->selectedIndex];
        if (item.action) item.action();
    }

    void navigateTo(MenuPage* subPage) {
        if (!subPage) return;
        menuStack.push_back(current);
        current = subPage;
        // 切换页面时，重置视觉索引
        visualIndex = (float)current->selectedIndex;
    }

    bool back() {
        if (menuStack.empty()) return false;
        current = menuStack.back();
        menuStack.pop_back();
        visualIndex = (float)current->selectedIndex;
        return true;
    }

    MenuPage* getCurrentPage() { return current; }

private:
    MenuPage* rootMenu = nullptr;
    MenuPage* current = nullptr;
    std::vector<MenuPage*> menuStack;
    
    // 新增：动画用的浮点数索引
    float visualIndex = 0.0;
};