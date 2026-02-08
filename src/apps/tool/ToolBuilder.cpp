#include "apps/tool/ToolBuilder.h"
#include "controller/AppController.h"
#include "assets/Lang.h"
#include "apps/tool/flashlight/Flashlight.h"
#include "apps/tool/gyroscope/Gyroscope.h"

// -- 构建函数 --------------------------------------------------------------------

MenuPage* ToolBuilder::build(AppController* sys) {
    int L = AppData.systemConfig.languageIndex;

    // 创建工具菜单页
    MenuPage* page = sys->createPage(STR_TOOLS[L], LAYOUT_LIST);

    // 返回按钮
    page->add(STR_BACK[L], nullptr, [sys]() {
        sys->menuCtrl.back();
    });

    // 手电筒
    page->add(STR_TOOL_FLASH[L], nullptr, [sys]() {
        sys->startApp(new Flashlight());
    });

    // 陀螺仪 (IMU)
    page->add(STR_TOOL_IMU[L], nullptr, [sys]() {
        sys->startApp(new Gyroscope());
    });

    return page;
}