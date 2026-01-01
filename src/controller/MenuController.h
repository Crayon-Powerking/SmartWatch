#pragma once
#include <vector>
#include "model/MenuTypes.h"

class MenuController {
public:
    // 初始化，设置根菜单
    void init(MenuPage* root) {
        rootMenu = root;
        menuStack.clear();
        current = rootMenu;
    }

    // [核心导航] 下一个 (右)
    void next() {
        if (!current) return;
        if (current->selectedIndex < current->items.size() - 1) {
            current->selectedIndex++;
        }
    }

    // [核心导航] 上一个 (左)
    void prev() {
        if (!current) return;
        if (current->selectedIndex > 0) {
            current->selectedIndex--;
        }
    }

    // [核心交互] 确认/进入
    void enter() {
        if (!current || current->items.empty()) return;
        MenuItem& item = current->items[current->selectedIndex];
        if (item.action) item.action();
    }

    // [核心交互] 进入子菜单 (被 action 调用)
    void navigateTo(MenuPage* subPage) {
        if (!subPage) return;
        menuStack.push_back(current); // 记录当前页
        current = subPage;            // 进子菜单
    }

    // [核心交互] 返回上一级
    // 返回 false 代表已经在根目录了，没法退了 (通知 main 切回表盘)
    bool back() {
        if (menuStack.empty()) return false;
        current = menuStack.back(); // 读档
        menuStack.pop_back();       // 销毁记录
        return true;
    }

    MenuPage* getCurrentPage() { return current; }

private:
    MenuPage* rootMenu = nullptr;
    MenuPage* current = nullptr;
    std::vector<MenuPage*> menuStack;
};