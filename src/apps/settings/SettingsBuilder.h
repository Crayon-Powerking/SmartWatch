#pragma once
#include "model/MenuTypes.h"

static const char* ICON_RENDER_STAR = (const char*)2;
class AppController;

class SettingsBuilder {
public:
    static MenuPage* build(AppController* app);

private:

};