#include "MenuFactory.h"
#include "assets/AppIcons.h" 
#include "assets/Lang.h"     

void MenuFactory::build(AppController* app) {
    app->destroyMenuTree();
    int lang = AppData.languageIndex;

    // ===========================
    // 1. 创建页面容器 & 指定布局
    // ===========================
    
    // 主菜单 -> 使用横向图标模式 (Carousel)
    app->rootMenu = app->createPage("Home", LAYOUT_ICON);
    
    // 设置页 -> 使用纵向列表模式 (List) - 默认就是 List，不传也可以，但为了清晰写上
    MenuPage* settings = app->createPage(STR_SETTINGS[lang], LAYOUT_LIST);

    // 游戏页 -> 使用纵向列表模式 (List)
    MenuPage* gamePage = app->createPage("Games", LAYOUT_LIST);

    // ===========================
    // 2. 填充内容
    // ===========================

    // --- 游戏页 (纯文本列表) ---
    gamePage->add("Tetris", [](){ Serial.println("Start Tetris"); });
    gamePage->add("Snake", [](){ Serial.println("Start Snake"); });
    gamePage->add("Space Invaders", [](){ Serial.println("Invaders"); });
    gamePage->add("Dino Run", [](){ Serial.println("Dino"); });
    gamePage->add("2048", [](){ Serial.println("2048"); });
    gamePage->add("Flappy Bird", [](){ Serial.println("Flappy"); });
    gamePage->add("Flappy Bird1234567891011121314", [](){ Serial.println("Flappy"); });

    // --- 设置页 (带图标的列表) ---
    // 列表模式下，如果有图标，PageVerticalMenu 会自动处理 (图标在左，文字在右)
    // 如果没有图标(nullptr)，文字就会靠左
    char langTitle[32];
    snprintf(langTitle, 32, "%s: %s", STR_LANG_LABEL[lang], STR_LANG_VAL[lang]);

    settings->add(langTitle, icon_setting, [app](){
        AppData.languageIndex = !AppData.languageIndex;
        app->storage.save();
        MenuFactory::build(app); 
        app->inMenuMode = true;
        app->menuCtrl.init(app->rootMenu);
    });
    settings->add(STR_REBOOT[lang], icon_tool, [](){ ESP.restart(); });

    // --- 主菜单 (横向图标) ---
    app->rootMenu->add(STR_SETTINGS[lang], icon_setting, [app, settings](){
        app->menuCtrl.enter(settings);
    });

    app->rootMenu->add(STR_WEATHER[lang], icon_weather, [app](){ 
        app->checkWeather(); 
    });

    // 链接到游戏页
    app->rootMenu->add(STR_GAME[lang], icon_game, [app, gamePage](){
        app->menuCtrl.enter(gamePage);
    });

    app->rootMenu->add(STR_CALENDAR[lang], icon_calendar, [](){});
    app->rootMenu->add(STR_ALARM[lang],    icon_alarm,    [](){});
    app->rootMenu->add(STR_TOOLS[lang],    icon_tool,     [](){});
    app->rootMenu->add(STR_ABOUT[lang],    icon_information, [](){});
    
    // 默认居中
    if (!app->rootMenu->items.empty()) {
        app->rootMenu->selectedIndex = app->rootMenu->items.size() / 2;
    }
}