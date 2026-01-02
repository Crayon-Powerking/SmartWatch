#include "SettingsBuilder.h"
#include "controller/AppController.h"
#include "controller/MenuFactory.h" 
#include "assets/AppIcons.h"
#include "assets/Lang.h"

MenuPage* SettingsBuilder::build(AppController* app) {
    int lang = AppData.languageIndex;
    
    // 1. 创建设置主页
    MenuPage* page = app->createPage(STR_SETTINGS[lang], LAYOUT_LIST);

    // ==========================================
    // 0. 顶部通用返回键
    // ==========================================
    page->add(STR_BACK[lang], nullptr, [app](){
        app->menuCtrl.back(); 
    });

    // ==========================================
    // 1. 语言切换 (极简版)
    // ==========================================
    char langTitle[32];
    snprintf(langTitle, 32, "%s: %s", STR_LANG_LABEL[lang], STR_LANG_VAL[lang]);

    page->add(langTitle, icon_setting, [app](){
        // 1. 修改数据
        AppData.languageIndex = !AppData.languageIndex;
        app->storage.save();
        
        // 2. 重建菜单 (因为文字变了)
        MenuFactory::build(app); 
        
        // 3. 重新定位 (为了用户体验，尽量回到原来的位置)
        // 由于 rebuild 后旧指针失效，我们重新初始化到设置页会比较麻烦
        // 最简单的逻辑：重建后直接回到主菜单，让用户重新进来看新语言效果
        // 如果想回到设置页，需要遍历 rootMenu 找到 setting 入口
        
        app->inMenuMode = true;
        app->menuCtrl.init(app->rootMenu);
        
        // 不需要 Toast 提示
    });

    return page;
}