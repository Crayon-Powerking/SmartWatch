#include "MenuFactory.h"
#include "assets/AppIcons.h" // 引入图标
#include "assets/Lang.h"     // 引入语言

void MenuFactory::build(AppController* app) {
    // 1. 先帮 AppController 清理旧垃圾
    app->destroyMenuTree();

    int lang = AppData.languageIndex;

    // 2. 创建页面 (直接操作 app 的内部变量，因为我们将设为 friend)
    app->rootMenu = app->createPage("Home");
    MenuPage* settings = app->createPage(STR_SETTINGS[lang]);

    // ===========================
    // 构建：设置页 (二级菜单)
    // ===========================
    char langTitle[32];
    snprintf(langTitle, 32, "%s: %s", STR_LANG_LABEL[lang], STR_LANG_VAL[lang]);

    settings->add(langTitle, icon_setting, [app](){
        // 点击切换语言
        AppData.languageIndex = !AppData.languageIndex;
        app->storage.save();
        // 重新构建菜单 (递归调用自己)
        MenuFactory::build(app); 
        // 重新进入菜单模式
        app->inMenuMode = true;
        app->menuCtrl.init(app->rootMenu);
        // 如果想直接回到设置页，可以加逻辑跳转，这里暂时回主页
    });

    settings->add(STR_REBOOT[lang], icon_tool, [](){ ESP.restart(); });

    // ===========================
    // 构建：主页 (一级菜单)
    // ===========================
    
    // 链接到设置页
    app->rootMenu->add(STR_SETTINGS[lang], icon_setting, [app, settings](){
        app->menuCtrl.enter(settings);
    });

    // 天气 (调用 app 的私有函数 checkWeather)
    app->rootMenu->add(STR_WEATHER[lang], icon_weather, [app](){ 
        app->checkWeather(); 
    });

    // 其他空功能
    app->rootMenu->add(STR_CALENDAR[lang], icon_calendar, [](){});
    app->rootMenu->add(STR_ALARM[lang],    icon_alarm,    [](){});
    app->rootMenu->add(STR_TOOLS[lang],    icon_tool,     [](){});
    
    // ===========================
    // 【核心需求实现 1/2】: 默认居中
    // ===========================
    // 无论什么时候构建完，都强制把光标放在中间
    if (!app->rootMenu->items.empty()) {
        app->rootMenu->selectedIndex = app->rootMenu->items.size() / 2;
    }
}