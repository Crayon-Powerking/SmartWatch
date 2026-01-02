#pragma once
#include "controller/AppController.h" 

// 只是声明一下，具体实现在 .cpp 里
class MenuFactory {
public:
    // 静态函数：传入 AppController 的指针，帮它把菜单建好
    static void build(AppController* app);
};