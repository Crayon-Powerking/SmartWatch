#include "MenuFactory.h"
#include "assets/AppIcons.h" 
#include "assets/Lang.h"    

#include "apps/games/GamesBuilder.h"
#include "apps/settings/SettingsBuilder.h" 
#include "apps/information/AboutApp.h"
#include "apps/weather/WeatherApp.h"

void MenuFactory::build(AppController* app) {
    app->destroyMenuTree();
    int lang = AppData.languageIndex;

    // ===========================
    // 1. 外包生产 (调用 Builders)
    // ===========================
    // 游戏页 -> 交给 GamesBuilder
    MenuPage* gamePage = GamesBuilder::build(app);
    
    // 设置页 -> 交给 SettingsBuilder
    MenuPage* settingsPage = SettingsBuilder::build(app); 
    
    // ===========================
    // 2. 总装车间 (组装 Home)
    // ===========================
    app->rootMenu = app->createPage("Home", LAYOUT_ICON);

    // --- 挂载 设置 ---
    app->rootMenu->add(STR_SETTINGS[lang], icon_setting, [app, settingsPage](){
        app->menuCtrl.enter(settingsPage);
    });

    // --- 挂载 天气 ---
    app->rootMenu->add(STR_WEATHER[lang], icon_weather, [app](){ 
        app->startApp(new WeatherApp(app));
    });

    // --- 挂载 游戏 ---
    app->rootMenu->add(STR_GAME[lang], icon_game, [app, gamePage](){
        app->menuCtrl.enter(gamePage);
    });

    // --- 挂载 其他 (占位) ---
    app->rootMenu->add(STR_CALENDAR[lang], icon_calendar, [](){});
    app->rootMenu->add(STR_ALARM[lang],    icon_alarm,    [](){});
    app->rootMenu->add(STR_TOOLS[lang],    icon_tool,     [](){});
    app->rootMenu->add(STR_ABOUT[lang], icon_information, [app](){
        app->startApp(new AboutApp(app));
    });
    
    // 默认选中中间
    if (!app->rootMenu->items.empty()) {
        app->rootMenu->selectedIndex = app->rootMenu->items.size() / 2;
    }
}