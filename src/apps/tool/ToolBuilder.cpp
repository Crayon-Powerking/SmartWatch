#include "ToolBuilder.h"
#include "controller/AppController.h"
#include "assets/Lang.h"
#include "apps/tool/flashlight/Flashlight.h"

MenuPage* ToolBuilder::build(AppController* sys){
    int L = AppData.systemConfig.languageIndex;
    MenuPage* page = sys->createPage(STR_TOOLS[L], LAYOUT_LIST);
    page->add(STR_BACK[L], nullptr, [sys](){ sys->menuCtrl.back(); });

    page->add(STR_TOOL_FLASH[L], nullptr, [sys](){
        sys->startApp(new Flashlight());
    });

    return page;
};