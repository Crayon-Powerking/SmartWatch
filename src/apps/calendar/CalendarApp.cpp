#include "apps/calendar/CalendarApp.h"
#include "controller/AppController.h"
#include "assets/Lang.h"
#include "assets/AppIcons.h"
#include <cstring>
#include <cstdio>

static const char** FORTUNE_LIST[] = {
    (const char**)STR_LUCK_0,
    (const char**)STR_LUCK_1,
    (const char**)STR_LUCK_2,
    (const char**)STR_LUCK_3,
    (const char**)STR_LUCK_4
};
static const int FORTUNE_COUNT = 5;

// -- 辅助函数 --------------------------------------------------------------------

void CalendarApp::loadCache() {
    bool success = sys->storage.loadStruct("cal_cache", cache);
    if (!success) {
        memset(&cache, 0, sizeof(CalendarCache));
        cache.dayStamp = 0;
        cache.isRevealed = false;
        cache.fortuneIndex = 0;
        strcpy(cache.holiday.name, "Loading...");
        strcpy(cache.holiday.date, "----");
    }
}

void CalendarApp::saveCache() {
    sys->storage.saveStruct("cal_cache", cache);
}

void CalendarApp::checkDayChange() {
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);

    // 格式化日期戳: YYYYMMDD
    int todayStamp = (timeinfo->tm_year + 1900) * 10000 + 
                     (timeinfo->tm_mon + 1) * 100 + 
                     timeinfo->tm_mday;

    if (timeinfo->tm_year + 1900 < 2025) return; 

    if (cache.dayStamp != todayStamp) {
        cache.dayStamp = todayStamp;
        cache.isRevealed = false;
        generateDailyFortune();
        needNetworkUpdate = true;
        saveCache();
    } else {
        if (!cache.holiday.success) needNetworkUpdate = true;
    }
}

void CalendarApp::generateDailyFortune() {
    srand(cache.dayStamp);
    cache.fortuneIndex = rand() % FORTUNE_COUNT;
}

const char* getLocalizedHolidayName(const char* apiName) {
    int L = AppData.systemConfig.languageIndex;
    if (strcmp(apiName, "元旦") == 0) return STR_HOL_NEWYEAR[L];
    if (strcmp(apiName, "春节") == 0) return STR_HOL_SPRING[L];
    if (strcmp(apiName, "清明节") == 0) return STR_HOL_TOMB[L];
    if (strcmp(apiName, "劳动节") == 0) return STR_HOL_LABOR[L];
    if (strcmp(apiName, "端午节") == 0) return STR_HOL_GRAGON[L];
    if (strcmp(apiName, "中秋节") == 0) return STR_HOL_MIDAUTUMN[L];
    if (strcmp(apiName, "国庆节") == 0) return STR_HOL_NATIONAL[L];
    return apiName;
}

// -- 按键处理 --------------------------------------------------------------------

void CalendarApp::onKeySelect() {
    this->isExiting = true;
}

void CalendarApp::onKeyUp() {
    // 预留：单击 Up 键的逻辑
}

// -- 生命周期 --------------------------------------------------------------------

void CalendarApp::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    this->lastFrameTime = millis();
    this->holdProgress = 0.0f;
    this->needNetworkUpdate = false;

    loadCache();
    checkDayChange();

    // 绑定按键，并使用 processInput() 过滤息屏误触
    sys->btnSelect.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeySelect();
    });

    sys->btnUp.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeyUp();
    });
}

int CalendarApp::onLoop() {
    if (isExiting) return 1;

    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;

    // 网络更新逻辑
    if (needNetworkUpdate && sys->network.isConnected()) {
        HolidayInfo newHoliday = sys->network.fetchNextHoliday();
        if (newHoliday.success) {
            this->cache.holiday = newHoliday;
            saveCache();
        }
        needNetworkUpdate = false;
    }

    // 蓄力逻辑 (通过 isPressed 轮询实现)
    if (!cache.isRevealed) {
        if (sys->btnUp.isPressed()) {
            holdProgress += 1.5f * dt;
            if (holdProgress >= 1.0f) {
                holdProgress = 1.0f;
                cache.isRevealed = true;
                saveCache();
            }
        } else {
            holdProgress -= 3.0f * dt;
            if (holdProgress < 0.0f) holdProgress = 0.0f;
        }
    } else {
        holdProgress = 0.0f;
    }

    render();
    return 0;
}

void CalendarApp::onExit() {
    saveCache();
}

// -- 绘图渲染 --------------------------------------------------------------------

void CalendarApp::render() {
    sys->display.clear();
    sys->display.setFontMode(1);
    sys->display.setDrawColor(1);

    renderHolidayInfo();
    renderFortuneReveal();
}

void CalendarApp::renderHolidayInfo() {
    int L = AppData.systemConfig.languageIndex;
    sys->display.drawLine(0, 32, 128, 32);
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);

    sys->display.drawText(2, 12, STR_UNTIL[L]);
    sys->display.drawText(2, 26, getLocalizedHolidayName(cache.holiday.name));

    time_t now = time(NULL);
    struct tm t_now;
    localtime_r(&now, &t_now);
    
    t_now.tm_hour = 0;
    t_now.tm_min = 0;
    t_now.tm_sec = 0;
    time_t todayMidnight = mktime(&t_now);

    long targetTs = cache.holiday.targetTs;

    long diff = targetTs - todayMidnight;
    int days = (diff + 10) / 86400;
    if (days < 0) days = 0;

    char buf[16];
    snprintf(buf, sizeof(buf), "%d", days);
    sys->display.setFont(u8g2_font_logisoso24_tn);
    int w = sys->display.getStrWidth(buf);
    sys->display.drawText(126 - w, 28, buf);
}

void CalendarApp::renderFortuneReveal() {
    int L = AppData.systemConfig.languageIndex;
    int BaseY = 34;

    if (cache.isRevealed) {
        sys->display.setFont(u8g2_font_wqy12_t_gb2312);
        sys->display.drawText(2, BaseY + 12, STR_CAL_TODAY[L]);

        int idx = cache.fortuneIndex;
        if (idx < 0 || idx >= FORTUNE_COUNT) idx = 0;
        const char* luckText = FORTUNE_LIST[idx][L];
        sys->display.drawText(2, BaseY + 26, luckText);
    } else {
        sys->display.setFont(u8g2_font_wqy12_t_gb2312);
        const char* holdText = STR_CAL_HOLD[L];
        int textW = sys->display.getStrWidth(holdText);
        sys->display.drawText((128 - textW) / 2, BaseY + 14, holdText);

        int barW = 100, barH = 8;
        int barX = (128 - barW) / 2, barY = BaseY + 18;
        sys->display.drawFrame(barX, barY, barW, barH);

        if (holdProgress > 0) {
            int fillW = (int)(holdProgress * (barW - 2));
            sys->display.drawBox(barX + 1, barY + 1, fillW, barH - 2);
        }
    }
}