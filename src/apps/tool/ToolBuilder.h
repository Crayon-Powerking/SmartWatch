#pragma once

#include "model/MenuTypes.h"

class AppController;

// -- 类定义 ----------------------------------------------------------------------
class ToolBuilder {
public:
    static MenuPage* build(AppController* sys);
};