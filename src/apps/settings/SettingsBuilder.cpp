#include "SettingsBuilder.h"
#include "controller/AppController.h"
#include "assets/appIcons.h"
#include "assets/Lang.h"

static const int MAX_LANG_COUNT = 2;
static const int MAX_CURSOR_COUNT = 2;
static const int MAX_SLEEP_COUNT = 5;

static const int SLEEP_OPTS[] = { 0, 15, 30, 60, 300 }; // 单位：秒

MenuPage* SettingsBuilder::build(AppController* sys) {
    int L = AppData.systemConfig.languageIndex;
    
    MenuPage* page = sys->createPage(STR_SETTINGS[L], LAYOUT_LIST);
    page->add(STR_BACK[L], nullptr, [sys](){sys->menuCtrl.back();});

    // 语言设置
    page->add(STR_LANG_LABEL[L], nullptr, [sys, page](){
        buildLanguagePage(sys, page);
    });

    // 光标样式设置
    page->add(STR_CURSOR[L], nullptr, [sys, page](){
        buildCursorPage(sys, page);
    });

    // 反色设置切换
    page->add(STR_INVERSE[L], nullptr, [sys, L](){
        AppData.systemConfig.colorinverse = !AppData.systemConfig.colorinverse;
        sys->storage.save();
    });

    // 睡眠时间设置
    page->add(STR_SLEEP_VAL[L], nullptr, [sys, page](){
        buildSleepPage(sys, page);
    });

    return page;
}

void SettingsBuilder::buildLanguagePage(AppController* sys, MenuPage* parent) {
    int L = AppData.systemConfig.languageIndex;
    MenuPage* subPage = sys->createPage("Select Language", LAYOUT_LIST);
    subPage->add(STR_BACK[L], nullptr, [sys](){sys->menuCtrl.back();});

    for (int i = 0; i < MAX_LANG_COUNT; i++) {
        // 添加具体语言项
        subPage->add(STR_LANG_VAL[i], nullptr, [sys, i](){
            // 点击逻辑
            if (AppData.systemConfig.languageIndex != i) {
                AppData.systemConfig.languageIndex = i; // 更改语言设置
                sys->storage.save();       // 保存到存储器
                sys->scheduleReload();     // 安排重载菜单
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
    subPage->add(STR_BACK[L], nullptr, [sys](){sys->menuCtrl.back();});
    for (int i = 0; i < MAX_CURSOR_COUNT; i++) {
        // 添加具体光标样式项
        const char* cursorName = (i == 0) ? STR_SOLID[L] : STR_HOLLOW[L];
        subPage->add(cursorName, nullptr, [sys, i](){
            // 点击逻辑
            if (AppData.systemConfig.cursorStyle != i) {
                AppData.systemConfig.cursorStyle = i; // 更改光标样式设置
                sys->storage.save();       // 保存到存储器
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
    subPage->add(STR_BACK[L], nullptr, [sys](){sys->menuCtrl.back();});
    for(int i = 0; i < MAX_SLEEP_COUNT; i++) {
        int val = SLEEP_OPTS[i];
        // 添加具体睡眠时间项
        subPage->add(STR_SLEEP[L][i], nullptr, [sys, val](){
            // 点击逻辑
            if (AppData.systemConfig.sleepTimeout != val) {
                AppData.systemConfig.sleepTimeout = val; // 更改睡眠时间设置
                sys->onButtonEvent();
                sys->storage.save();
                sys->menuCtrl.back();
            } else {
                sys->menuCtrl.back();
            }
        });
    }
    sys->menuCtrl.enter(subPage);
}