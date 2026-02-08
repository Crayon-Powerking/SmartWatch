#pragma once

#include "model/MenuTypes.h"

class AppController;

// -- 类定义 ----------------------------------------------------------------------
class GamesBuilder {
public:
    static MenuPage* build(AppController* sys);
};