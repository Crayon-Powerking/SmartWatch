#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>

// 1. 通用回调动作
using MenuCallback = std::function<void()>;

// 2. 单个菜单项
struct MenuItem {
    String title;        // 名字 (如 "Settings")
    const uint8_t* icon; // 图标 (位图数据)
    MenuCallback action; // 具体的动作 (进入子菜单 或 执行功能)
    
    // 构造函数
    MenuItem(String t, const uint8_t* i, MenuCallback cb) 
        : title(t), icon(i), action(cb) {}
};

// 3. 菜单页 (包含一组项目)
struct MenuPage {
    String title;                // 页标题 (如 "Main")
    std::vector<MenuItem> items; // 这一页的所有图标
    int selectedIndex = 0;       // 记住这一页选到了第几个
    
    MenuPage(String t) : title(t) {}
    
    void add(String title, const uint8_t* icon, MenuCallback cb) {
        items.push_back(MenuItem(title, icon, cb));
    }
};