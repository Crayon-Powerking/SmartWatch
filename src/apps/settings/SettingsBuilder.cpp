#include "apps/settings/SettingsBuilder.h"
#include "controller/AppController.h"
#include "assets/Lang.h"

// -- 常量定义 --------------------------------------------------------------------
static const int MAX_LANG_COUNT = 2;
static const int MAX_CURSOR_COUNT = 2;
static const int MAX_SLEEP_COUNT = 5;

// 单位：秒
static const int SLEEP_OPTS[] = { 0, 15, 30, 60, 300 };

// -- 主构建函数 ------------------------------------------------------------------

MenuPage* SettingsBuilder::build(AppController* sys) {
    int L = AppData.systemConfig.languageIndex;

    MenuPage* page = sys->createPage(STR_SETTINGS[L], LAYOUT_LIST);

    // 1. 返回按钮
    page->add(STR_BACK[L], nullptr, [sys]() {
        sys->menuCtrl.back();
    });

    // 2. 语言设置
    page->add(STR_LANG_LABEL[L], nullptr, [sys, page]() {
        buildLanguagePage(sys, page);
    });

    // 3. 光标样式
    page->add(STR_CURSOR[L], nullptr, [sys, page]() {
        buildCursorPage(sys, page);
    });

    // 4. 反色设置
    page->add(STR_INVERSE[L], nullptr, [sys, L]() {
        AppData.systemConfig.colorinverse = !AppData.systemConfig.colorinverse;
        sys->storage.save();
    });

    // 5. 睡眠时间
    page->add(STR_SLEEP_VAL[L], nullptr, [sys, page]() {
        buildSleepPage(sys, page);
    });

    return page;
}

// -- 子菜单构建函数 --------------------------------------------------------------

void SettingsBuilder::buildLanguagePage(AppController* sys, MenuPage* parent) {
    int L = AppData.systemConfig.languageIndex;
    MenuPage* subPage = sys->createPage("Select Language", LAYOUT_LIST);

    subPage->add(STR_BACK[L], nullptr, [sys]() {
        sys->menuCtrl.back();
    });

    for (int i = 0; i < MAX_LANG_COUNT; i++) {
        subPage->add(STR_LANG_VAL[i], nullptr, [sys, i]() {
            if (AppData.systemConfig.languageIndex != i) {
                AppData.systemConfig.languageIndex = i;
                sys->storage.save();
                sys->scheduleReload(); // 语言变更，重载菜单
            } else {
                sys->menuCtrl.back();
            }
        });
    }
    sys->menuCtrl.enter(subPage);
}

void SettingsBuilder::buildCursorPage(AppController* sys, MenuPage* parent) {
    int L = AppData.systemConfig.languageIndex;
    MenuPage* subPage = sys->createPage("Select Cursor", LAYOUT_LIST);

    subPage->add(STR_BACK[L], nullptr, [sys]() {
        sys->menuCtrl.back();
    });

    for (int i = 0; i < MAX_CURSOR_COUNT; i++) {
        const char* cursorName = (i == 0) ? STR_SOLID[L] : STR_HOLLOW[L];
        
        subPage->add(cursorName, nullptr, [sys, i]() {
            if (AppData.systemConfig.cursorStyle != i) {
                AppData.systemConfig.cursorStyle = i;
                sys->storage.save();
                sys->menuCtrl.back();
            } else {
                sys->menuCtrl.back();
            }
        });
    }
    sys->menuCtrl.enter(subPage);
}

void SettingsBuilder::buildSleepPage(AppController* sys, MenuPage* parent) {
    int L = AppData.systemConfig.languageIndex;
    MenuPage* subPage = sys->createPage("Select Sleep Time", LAYOUT_LIST);

    subPage->add(STR_BACK[L], nullptr, [sys]() {
        sys->menuCtrl.back();
    });

    for (int i = 0; i < MAX_SLEEP_COUNT; i++) {
        int val = SLEEP_OPTS[i];

        subPage->add(STR_SLEEP[L][i], nullptr, [sys, val]() {
            if (AppData.systemConfig.sleepTimeout != val) {
                AppData.systemConfig.sleepTimeout = val;
                sys->processInput();
                sys->storage.save();
                sys->menuCtrl.back();
            } else {
                sys->menuCtrl.back();
            }
        });
    }
    sys->menuCtrl.enter(subPage);
}