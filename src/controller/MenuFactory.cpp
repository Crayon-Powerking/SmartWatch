#include "MenuFactory.h"
#include "assets/AppIcons.h" 
#include "assets/Lang.h"    

#include "apps/games/GamesBuilder.h"
#include "apps/settings/SettingsBuilder.h" 
#include "apps/information/AboutApp.h"
#include "apps/weather/WeatherApp.h"
#include "apps/calendar/CalendarApp.h"
#include "apps/tool/ToolBuilder.h"
#include "apps/alarm/AlarmApp.h"

void MenuFactory::build(AppController* sys) {
    sys->destroyMenuTree();                                 // 先销毁旧菜单树
    int lang = AppData.systemConfig.languageIndex;                       // 当前语言索引

    MenuPage* gamePage = GamesBuilder::build(sys);          // 游戏页 -> 交给 GamesBuilder
    MenuPage* settingsPage = SettingsBuilder::build(sys);   // 设置页 -> 交给 SettingsBuilder
    MenuPage* toolsPage = ToolBuilder::build(sys);        // 工具页 -> 交给 ToolBuilder
    
    sys->rootMenu = sys->createPage("Home", LAYOUT_ICON);   // 根菜单，图标布局

    //----------------- 添加各个一级菜单项 ----------------//
    // 退出菜单
    sys->rootMenu->add(STR_Exit[lang],     icon_exit,     [sys](){
        sys->inMenuMode = false;
    });

    // 设置应用
    sys->rootMenu->add(STR_SETTINGS[lang], icon_setting, [sys, settingsPage](){
        sys->menuCtrl.enter(settingsPage);
    });

    // 天气应用
    sys->rootMenu->add(STR_WEATHER[lang], icon_weather, [sys](){ 
        sys->startApp(new WeatherApp());
    });

    // 游戏应用
    sys->rootMenu->add(STR_GAME[lang], icon_game, [sys, gamePage](){
        sys->menuCtrl.enter(gamePage);
    });

    // 日历应用
    sys->rootMenu->add(STR_CALENDAR[lang], icon_calendar, [sys](){
        sys->startApp(new CalendarApp());
    });

    // 闹钟应用
    sys->rootMenu->add(STR_ALARM[lang], icon_alarm, [sys](){
        sys->startApp(new AlarmApp());
    });
    
    // 工具应用
    sys->rootMenu->add(STR_TOOLS[lang],    icon_tool,     [sys, toolsPage](){
        sys->menuCtrl.enter(toolsPage);
    });

    // 关于应用
    sys->rootMenu->add(STR_ABOUT[lang],    icon_information, [sys](){
        sys->startApp(new AboutApp());
    });

    
    
    //初始选择居中
    if (!sys->rootMenu->items.empty()) {
        sys->rootMenu->selectedIndex = sys->rootMenu->items.size() / 2;
    }
}