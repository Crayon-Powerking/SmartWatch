#pragma once
#include "controller/AppController.h" 

class MenuFactory {
public:
    static void build(AppController* app);  // 构建菜单树
};