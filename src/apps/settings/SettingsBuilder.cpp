#include "apps/settings/SettingsBuilder.h"
#include "controller/AppController.h"
#include "assets/Lang.h"
#include <cstdio>
#include <WiFi.h>

// -- 常量定义 --------------------------------------------------------------------
static const int MAX_LANG_COUNT = 2;
static const int MAX_CURSOR_COUNT = 2;
static const int MAX_SLEEP_COUNT = 5;

// 单位：秒
static const int SLEEP_OPTS[] = { 0, 15, 30, 60, 300 };
static char sleepMenuItemTitle[32];

// -- 辅助函数 --------------------------------------------------------------------
String getSleepLabel(int sec, int lang) {
    for (int i = 0; i < MAX_SLEEP_COUNT; i++) {
        if (SLEEP_OPTS[i] == sec) return String(STR_SLEEP[lang][i]);
    }
    return "?";
}

// -- 主构建函数 ------------------------------------------------------------------

MenuPage* SettingsBuilder::build(AppController* sys) {
    int L = AppData.systemConfig.languageIndex;

    MenuPage* page = sys->createPage(STR_SETTINGS[L], LAYOUT_LIST);

    int currentSec = AppData.systemConfig.sleepTimeout;
    int targetIndex = 0;
    for (int i = 0; i < MAX_SLEEP_COUNT; i++) {
        if (SLEEP_OPTS[i] == currentSec) {
            targetIndex = i;
            break;
        }
    }
    snprintf(sleepMenuItemTitle, sizeof(sleepMenuItemTitle), "%s: %s", STR_SLEEP_VAL[L], STR_SLEEP[L][targetIndex]);

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
    String currentTitle = String(STR_SLEEP_VAL[L]) + ": " + getSleepLabel(AppData.systemConfig.sleepTimeout, L);
    page->add(currentTitle, nullptr, [sys, page]() {
        buildSleepPage(sys, page);
    });


    // 6. WIFI 设置
    bool isWifiOn = (WiFi.getMode() != WIFI_OFF);
    const char* wifititle = isWifiOn ? STR_WIFI_ON[L] : STR_WIFI_OFF[L];
    page->add(wifititle, nullptr, [sys, page, L]() {
        bool currentOn = (WiFi.getMode() != WIFI_OFF);
        if (currentOn) {
            sys->network.stop(); 
        } else {
            sys->network.begin(); 
            sys->network.connect(AppData.userConfig.wifi_ssid, AppData.userConfig.wifi_pass);
        }
        if (page) {
            for (auto &item : page->items) {
                if (item.title == STR_WIFI_ON[L] || item.title == STR_WIFI_OFF[L]) {
                    item.title = currentOn ? STR_WIFI_OFF[L] : STR_WIFI_ON[L];
                    break;
                }
            }
        }
    });

    // 7. 蓝牙设置
    bool isBLEOn = sys->ble.isServiceRunning();
    const char* bleTitle = isBLEOn ? STR_BLE_ON[L] : STR_BLE_OFF[L];
    page->add(bleTitle, nullptr, [sys, page, L]() {
        bool currentOn = sys->ble.isServiceRunning();
        if (currentOn) {
            sys->ble.stop();
        } else {
            sys->ble.begin("ESP32-Watch");
        }
        if (page) {
            for (auto &item : page->items) {
                if (item.title == STR_BLE_ON[L] || item.title == STR_BLE_OFF[L]) {
                    item.title = currentOn ? STR_BLE_OFF[L] : STR_BLE_ON[L];
                    break;
                }
            }
        }
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
        subPage->add(STR_SLEEP[L][i], nullptr, [sys, parent, val, i, L]() {
            if (AppData.systemConfig.sleepTimeout != val) {
                AppData.systemConfig.sleepTimeout = val;
                sys->storage.save();
                if (parent) {
                    String prefix = String(STR_SLEEP_VAL[L]); 
                    for (auto &item : parent->items) {
                        if (item.title.startsWith(prefix)) {
                            item.title = prefix + ": " + String(STR_SLEEP[L][i]);
                            break; 
                        }
                    }
                }
                sys->menuCtrl.back();
            } else {
                sys->menuCtrl.back();
            }
        });
    }
    sys->menuCtrl.enter(subPage);
}