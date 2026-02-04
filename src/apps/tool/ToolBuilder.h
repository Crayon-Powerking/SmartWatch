#pragma once
#include "model/MenuTypes.h"

class AppController;

class ToolBuilder{
public:
    static MenuPage* build(AppController* sys);
};