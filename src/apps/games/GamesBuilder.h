#pragma once
// 引入必要的类型定义
#include "model/MenuTypes.h"

// 前向声明，告诉编译器 AppController 是个类，别报错
class AppController;

class GamesBuilder {
public:
    // 对外只暴露这一个静态函数：给我控制器，我返给你做好的游戏菜单
    static MenuPage* build(AppController* app);
};