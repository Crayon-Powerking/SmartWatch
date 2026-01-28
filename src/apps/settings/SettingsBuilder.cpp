#include "SettingsBuilder.h"
#include "controller/AppController.h"
#include "assets/AppIcons.h"
#include "assets/Lang.h"

// 语言总数
static const int MAX_LANG_COUNT = 2;

MenuPage* SettingsBuilder::build(AppController* app) {
    int currentLang = AppData.languageIndex;
    
    MenuPage* page = app->createPage(STR_SETTINGS[currentLang], LAYOUT_LIST);
    page->add(STR_BACK[currentLang], nullptr, [app](){
        app->menuCtrl.back(); 
    });

    page->add(STR_LANG_LABEL[currentLang], icon_setting, [app, currentLang](){
        
        MenuPage* subPage = app->createPage("Select Language", LAYOUT_LIST);
        subPage->add(STR_BACK[currentLang], nullptr, [app](){
            app->menuCtrl.back(); // 回到上一级
        });

        for (int i = 0; i < MAX_LANG_COUNT; i++) {
            // 添加具体语言项
            subPage->add(STR_LANG_VAL[i], nullptr, [app, i](){
                
                // 点击逻辑
                if (AppData.languageIndex != i) {
                    AppData.languageIndex = i; // 更改语言设置
                    app->storage.save();       // 保存到存储器
                    app->scheduleReload();     // 安排重载菜单
                } else {
                    app->menuCtrl.back();
                }
            });
        }
        app->menuCtrl.enter(subPage);
    });

    return page;
}