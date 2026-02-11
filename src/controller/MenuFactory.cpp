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

// -- 菜单构建 --------------------------------------------------------------------
void MenuFactory::build(AppController* sys) {
    sys->destroyMenuTree();
    int lang = AppData.systemConfig.languageIndex;

    // 1. 构建子页面
    MenuPage* gamePage = GamesBuilder::build(sys);
    MenuPage* settingsPage = SettingsBuilder::build(sys);
    MenuPage* toolsPage = ToolBuilder::build(sys);
    
    // 2. 构建根菜单 (图标布局)
    sys->rootMenu = sys->createPage("Home", LAYOUT_ICON);

    // -- 添加一级菜单项 ----------------------------------------------------------
    
    // 退出
    sys->rootMenu->add(STR_Exit[lang], icon_exit, [sys](){ sys->inMenuMode = false; });

    // 设置
    sys->rootMenu->add(STR_SETTINGS[lang], icon_setting, [sys, settingsPage](){ sys->menuCtrl.enter(settingsPage); });

    // 天气 (独立App)
    sys->rootMenu->add(STR_WEATHER[lang], icon_weather, [sys](){ sys->startApp(new WeatherApp()); });

    // 游戏 (子菜单)
    sys->rootMenu->add(STR_GAME[lang], icon_game, [sys, gamePage](){ sys->menuCtrl.enter(gamePage); });

    // 日历
    sys->rootMenu->add(STR_CALENDAR[lang], icon_calendar, [sys](){ sys->startApp(new CalendarApp()); });

    // 闹钟
    sys->rootMenu->add(STR_ALARM[lang], icon_alarm, [sys](){ sys->startApp(new AlarmApp()); });

    // 工具
    sys->rootMenu->add(STR_TOOLS[lang], icon_tool, [sys, toolsPage](){ sys->menuCtrl.enter(toolsPage); });

    // 关于
    sys->rootMenu->add(STR_ABOUT[lang], icon_information, [sys](){ sys->startApp(new AboutApp()); });

    // 3. 初始焦点居中
    if (!sys->rootMenu->items.empty()) {
        sys->rootMenu->selectedIndex = sys->rootMenu->items.size() / 2;
    }
}