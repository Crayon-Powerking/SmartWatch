#include "GamesBuilder.h"
#include "controller/AppController.h"
#include "assets/Lang.h" // 引入字典

#include "apps/games/dino/GameDino.h"

MenuPage* GamesBuilder::build(AppController* app) {
    
    int lang = AppData.languageIndex;
    MenuPage* page = app->createPage(STR_GAME[lang], LAYOUT_LIST);
    page->add(STR_BACK[lang], nullptr, [app](){ app->menuCtrl.back(); });

    // ==========================================
    // 1. 实际开发的游戏 (Priority)
    // ==========================================
    
    // Dino Run
    page->add(STR_GAME_DINO[lang], nullptr, [app](){ 
        app->startApp(new GameDino(app)); 
    });

    // 2048
    page->add(STR_GAME_2048[lang], [](){ 
        Serial.println("Start 2048"); 
        // app->startApp(new Game2048());
    });

    // ==========================================
    // 2. 占位游戏列表 (展示用)
    // ==========================================
    
    page->add(STR_GAME_TETRIS[lang],  [](){ Serial.println("Tetris"); });
    page->add(STR_GAME_SNAKE[lang],   [](){ Serial.println("Snake"); });
    page->add(STR_GAME_INVADER[lang], [](){ Serial.println("Invaders"); });
    page->add(STR_GAME_FLAPPY[lang],  [](){ Serial.println("Flappy"); });
    page->add(STR_GAME_PONG[lang],    [](){ Serial.println("Pong"); });
    page->add(STR_GAME_PACMAN[lang],  [](){ Serial.println("Pacman"); });
    page->add(STR_GAME_MINECRAFT[lang], [](){ Serial.println("MC"); });
    page->add(STR_GAME_DOOM[lang],      [](){ Serial.println("Doom"); });
    page->add(STR_GAME_GENSHIN[lang],   [](){ Serial.println("Genshin"); });

    // 测试长文本滚动
    page->add(STR_GAME_CYBER[lang], [](){ Serial.println("Wake up Samurai"); });

    return page;
}