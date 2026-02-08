#include "apps/games/GamesBuilder.h"
#include "controller/AppController.h"
#include "assets/Lang.h"
#include "apps/games/dino/GameDino.h"

// -- 构建函数 --------------------------------------------------------------------

MenuPage* GamesBuilder::build(AppController* sys) {
    int lang = AppData.systemConfig.languageIndex;
    
    // 创建菜单页
    MenuPage* page = sys->createPage(STR_GAME[lang], LAYOUT_LIST);

    // 返回按钮
    page->add(STR_BACK[lang], nullptr, [sys]() {
        sys->menuCtrl.back();
    });

    // -- 游戏列表 --

    // 小恐龙 (已实现)
    page->add(STR_GAME_DINO[lang], nullptr, [sys]() {
        sys->startApp(new GameDino());
    });

    // 占位符游戏 (未实现)
    page->add(STR_GAME_2048[lang], []() {
        Serial.println("Start 2048");
    });

    page->add(STR_GAME_TETRIS[lang], []() {
        Serial.println("Tetris");
    });

    page->add(STR_GAME_SNAKE[lang], []() {
        Serial.println("Snake");
    });

    page->add(STR_GAME_INVADER[lang], []() {
        Serial.println("Invaders");
    });

    page->add(STR_GAME_FLAPPY[lang], []() {
        Serial.println("Flappy");
    });

    page->add(STR_GAME_PONG[lang], []() {
        Serial.println("Pong");
    });

    page->add(STR_GAME_PACMAN[lang], []() {
        Serial.println("Pacman");
    });

    page->add(STR_GAME_MINECRAFT[lang], []() {
        Serial.println("MC");
    });

    page->add(STR_GAME_DOOM[lang], []() {
        Serial.println("Doom");
    });

    page->add(STR_GAME_CYBER[lang], []() {
        Serial.println("Wake up Samurai");
    });

    page->add(STR_GAME_GENSHIN[lang], []() {
        Serial.println("原神nb");
    });

    return page;
}