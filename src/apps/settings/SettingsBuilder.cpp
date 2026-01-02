#include "SettingsBuilder.h"
#include "controller/AppController.h"
#include "controller/MenuFactory.h" 
#include "assets/AppIcons.h"
#include "assets/Lang.h"

MenuPage* SettingsBuilder::build(AppController* app) {
    int lang = AppData.languageIndex;
    
    MenuPage* page = app->createPage(STR_SETTINGS[lang], LAYOUT_LIST);

    // 返回键
    page->add(STR_BACK[lang], nullptr, [app](){
        app->menuCtrl.back(); 
    });

    // 语言切换
    char langTitle[32];
    snprintf(langTitle, 32, "%s: %s", STR_LANG_LABEL[lang], STR_LANG_VAL[lang]);

    page->add(langTitle, icon_setting, [app](){
        // 1. 修改数据
        AppData.languageIndex = !AppData.languageIndex;
        app->storage.save();
        
        // 2. 【关键修复】请求安全重载
        // 不要在这里直接 build，而是告诉主循环：“下一帧帮我重载一下”
        // 这样当前函数可以安全返回，不会导致内存错误。
        app->scheduleReload(); 
    });

    return page;
}