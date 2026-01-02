#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>

// 1. 定义布局枚举 (新增)
enum MenuLayout {
    LAYOUT_ICON, // 横向图标 (一级菜单)
    LAYOUT_LIST  // 纵向列表 (二级菜单)
};

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
    
    // 2. 新增成员变量：该页面的布局类型
    MenuLayout layout; 

    // 构造函数增加默认值 (默认为列表，因为列表更常用)
    MenuPage(String t, MenuLayout l = LAYOUT_LIST) 
        : title(t), layout(l) {}

    MenuPage(String t) : title(t) {}
    
    void add(String title, const uint8_t* icon, MenuCallback cb) {
        items.push_back(MenuItem(title, icon, cb));
    }

    // 纯文本添加函数的重载
    void add(String title, MenuCallback cb) {
        items.push_back(MenuItem(title, nullptr, cb));
    }
};