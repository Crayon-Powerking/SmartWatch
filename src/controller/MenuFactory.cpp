#include "MenuFactory.h"
#include "assets/AppIcons.h" 
#include "assets/Lang.h"    

#include "apps/games/GamesBuilder.h"
#include "apps/settings/SettingsBuilder.h" 
#include "apps/information/AboutApp.h"
#include "apps/weather/WeatherApp.h"

void MenuFactory::build(AppController* app) {
    app->destroyMenuTree();                                 // 先销毁旧菜单树
    int lang = AppData.languageIndex;                       // 当前语言索引

    MenuPage* gamePage = GamesBuilder::build(app);          // 游戏页 -> 交给 GamesBuilder
    MenuPage* settingsPage = SettingsBuilder::build(app);   // 设置页 -> 交给 SettingsBuilder
    
    app->rootMenu = app->createPage("Home", LAYOUT_ICON);   // 根菜单，图标布局

    //----------------- 添加各个一级菜单项 ----------------//
    // 退出菜单
    app->rootMenu->add(STR_Exit[lang],     icon_exit,     [app](){
        app->inMenuMode = false;
    });

    // 设置应用
    app->rootMenu->add(STR_SETTINGS[lang], icon_setting, [app, settingsPage](){
        app->menuCtrl.enter(settingsPage);
    });

    // 天气应用
    app->rootMenu->add(STR_WEATHER[lang], icon_weather, [app](){ 
        app->startApp(new WeatherApp());
    });

    // 游戏应用
    app->rootMenu->add(STR_GAME[lang], icon_game, [app, gamePage](){
        app->menuCtrl.enter(gamePage);
    });

    app->rootMenu->add(STR_CALENDAR[lang], icon_calendar, [](){});
    app->rootMenu->add(STR_ALARM[lang],    icon_alarm,    [](){});
    app->rootMenu->add(STR_TOOLS[lang],    icon_tool,     [](){});

    // 关于应用
    app->rootMenu->add(STR_ABOUT[lang],    icon_information, [app](){
        app->startApp(new AboutApp());
    });

    
    
    //初始选择居中
    if (!app->rootMenu->items.empty()) {
        app->rootMenu->selectedIndex = app->rootMenu->items.size() / 2;
    }
}