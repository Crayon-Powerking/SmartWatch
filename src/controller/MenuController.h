#pragma once
#include <vector>
#include <Arduino.h>
#include "model/MenuTypes.h"

class MenuController {
public:
    void init(MenuPage* root) {
        rootMenu = root;                                           // 设置根菜单
        menuStack.clear();                                         // 清空导航栈
        current = rootMenu;                                        // 设置当前页面为根菜单
        if (current) visualIndex = (float)current->selectedIndex;  // 初始化视觉索引
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

    // [按键触发] 进入当前选中的子菜单或执行动作
    void enter() {
        if (!current || current->items.empty()) return;
        MenuItem& item = current->items[current->selectedIndex];
        if (item.action) item.action();
    }

    // 强制跳转到指定页面
    void enter(MenuPage* subPage) {
        navigateTo(subPage);
    }

    // 底层跳转实现
    void navigateTo(MenuPage* subPage) {
        if (!subPage) return;
        
        // 只有当 subPage 不是当前页面时才压栈，防止重复
        if (current != subPage) {
            menuStack.push_back(current);
        }
        
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
    
    // 动画用的浮点数索引
    float visualIndex = 0.0;
};