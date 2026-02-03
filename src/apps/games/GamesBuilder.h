#pragma once
#include "model/MenuTypes.h"

class AppController;

class GamesBuilder {
public:
    static MenuPage* build(AppController* sys);
};