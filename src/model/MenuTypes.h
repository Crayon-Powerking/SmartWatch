#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>

// -- 菜单枚举与回调 --------------------------------------------------------------
enum MenuLayout {
    LAYOUT_ICON,                    // 横向图标布局 (一级菜单)
    LAYOUT_LIST                     // 纵向列表布局 (二级菜单)
};

using MenuCallback = std::function<void()>;

// -- 菜单单项定义 ----------------------------------------------------------------
struct MenuItem {
    String title;                   // 显示名称
    const uint8_t* icon;            // 图标位图指针
    MenuCallback action;            // 点击触发的回调
    
    MenuItem(String t, const uint8_t* i, MenuCallback cb) 
        : title(t), icon(i), action(cb) {}
};

// -- 菜单页面定义 ----------------------------------------------------------------
struct MenuPage {
    String title;                   // 页面标题
    std::vector<MenuItem> items;    // 包含的菜单项列表
    int selectedIndex = 0;          // 记录当前选中的索引
    MenuLayout layout;              // 页面布局样式

    MenuPage(String t, MenuLayout l = LAYOUT_LIST) 
        : title(t), layout(l) {}
    
    // -- 动态操作 ----------------------------------------------------------------
    void add(String title, const uint8_t* icon, MenuCallback cb) {
        items.push_back(MenuItem(title, icon, cb));
    }

    void add(String title, MenuCallback cb) {
        items.push_back(MenuItem(title, nullptr, cb));
    }
};