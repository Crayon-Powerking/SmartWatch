#include "apps/calendar/CalendarApp.h"
#include "controller/AppController.h" 
#include "assets/Lang.h"
#include "assets/AppIcons.h"

// 引用外部对象
extern DisplayHAL display;
extern InputHAL btnSelect;
extern InputHAL btnUp;

static const char** FORTUNE_LIST[] = {
    (const char**)STR_LUCK_0, // 大吉
    (const char**)STR_LUCK_1, // 中吉
    (const char**)STR_LUCK_2, // 小吉
    (const char**)STR_LUCK_3, // 无Bug
    (const char**)STR_LUCK_4  // 忌上线
};
static const int FORTUNE_COUNT = 5;

void CalendarApp::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    this->lastFrameTime = millis();
    this->holdProgress = 0.0f;
    this->needNetworkUpdate = false;

    loadCache();
    checkDayChange();

    btnSelect.attachClick([this](){
        this->isExiting = true;
    });
    
    btnUp.attachClick([](){}); 
}

void CalendarApp::onExit() {
    saveCache();
}

int CalendarApp::onLoop() {
    if(isExiting) return 1;

    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;

    // --- 联网刷新节日逻辑 (非阻塞) ---
    if (needNetworkUpdate && sys->network.isConnected()) {
        HolidayInfo newHoliday = sys->network.fetchNextHoliday();
        if (newHoliday.success) {
            this->cache.holiday = newHoliday;
            saveCache();
        }
        needNetworkUpdate = false; 
    }

    // --- 蓄力交互逻辑 ---
    if (!cache.isRevealed) {
        if (btnUp.isPressed()) {
            holdProgress += 1.5f * dt; 
            if (holdProgress >= 1.0f) {
                holdProgress = 1.0f;
                cache.isRevealed = true;
                saveCache(); 
                // sys->vibrate(); // 如果有震动马达可以在这里调用
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

void CalendarApp::loadCache() {
    bool success = sys->storage.loadStruct("cal_cache", cache);
    if (!success) {
        memset(&cache, 0, sizeof(CalendarCache));
        cache.dayStamp = 0;
        cache.isRevealed = false;
        cache.fortuneIndex = 0; // 默认给个0
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
    
    // 简单的整数日期戳: 20260130
    int todayStamp = (timeinfo->tm_year + 1900) * 10000 + (timeinfo->tm_mon + 1) * 100 + timeinfo->tm_mday;

    if (timeinfo->tm_year + 1900 < 2025) return; // 时间未同步，跳过

    if (cache.dayStamp != todayStamp) {
        // === 新的一天 ===
        cache.dayStamp = todayStamp;
        cache.isRevealed = false; 
        
        generateDailyFortune();
        needNetworkUpdate = true; // 跨天了，顺便检查下节日有没有过期的
        
        saveCache();
    } else {
        // === 同一天 ===
        if (!cache.holiday.success) needNetworkUpdate = true;
    }
}

void CalendarApp::generateDailyFortune() {
    // 确定性随机算法：日期不变，结果不变
    int seed = cache.dayStamp; 
    srand(seed); 
    cache.fortuneIndex = rand() % FORTUNE_COUNT; // 从5个文案中抽一个
}

// 辅助函数：根据 API 返回的中文名，映射到本地的双语常量
const char* getLocalizedHolidayName(const char* apiName) {
    int L = AppData.systemConfig.languageIndex;
    
    // 字符串匹配
    if (strcmp(apiName, "元旦") == 0)   return STR_HOL_NEWYEAR[L];
    if (strcmp(apiName, "春节") == 0)   return STR_HOL_SPRING[L];
    if (strcmp(apiName, "清明节") == 0) return STR_HOL_TOMB[L];
    if (strcmp(apiName, "劳动节") == 0) return STR_HOL_LABOR[L];
    if (strcmp(apiName, "端午节") == 0) return STR_HOL_GRAGON[L];
    if (strcmp(apiName, "中秋节") == 0) return STR_HOL_MIDAUTUMN[L];
    if (strcmp(apiName, "国庆节") == 0) return STR_HOL_NATIONAL[L];
    return apiName; 
}

void CalendarApp::render() {
    display.clear();
    display.setFontMode(1);
    display.setDrawColor(1);
    
    renderHolidayInfo();
    renderFortuneReveal();

    display.update();
}

void CalendarApp::renderHolidayInfo() {
    int L = AppData.systemConfig.languageIndex;
    
    // 分割线
    display.drawLine(0, 32, 128, 32);

    display.setFont(u8g2_font_wqy12_t_gb2312);
    // 使用 Lang: "距离" / "Until Days"
    display.drawText(2, 12, STR_UNTIL[L]); 
    // 显示节日名
    display.drawText(2, 26, getLocalizedHolidayName(cache.holiday.name));
    
    // 计算天数
    time_t now = time(NULL);
    long diff = cache.holiday.targetTs - now;
    int days = diff / 86400;
    if (days < 0) days = 0;
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", days);
    
    display.setFont(u8g2_font_logisoso24_tn);
    int w = display.getStrWidth(buf);
    display.drawText(126 - w, 28, buf);
}

void CalendarApp::renderFortuneReveal() {
    int L = AppData.systemConfig.languageIndex;
    int BaseY = 34; 
    
    if (cache.isRevealed) {
        // === 已揭晓 ===
        display.setFont(u8g2_font_wqy12_t_gb2312);
        // 使用 Lang: "Today's Fortune:" / "今日运势:"
        display.drawText(2, BaseY + 12, STR_CAL_TODAY[L]); 
        
        // 获取对应的运势文字
        int idx = cache.fortuneIndex;
        if (idx < 0 || idx >= FORTUNE_COUNT) idx = 0; // 边界保护
        const char* luckText = FORTUNE_LIST[idx][L];
        display.drawText(2, BaseY + 26, luckText);
        
    } else {
        // === 未揭晓 (蓄力界面) ===
        display.setFont(u8g2_font_wqy12_t_gb2312);
        
        // 居中计算
        const char* holdText = STR_CAL_HOLD[L];
        int textW = display.getStrWidth(holdText);
        display.drawText((128 - textW) / 2, BaseY + 14, holdText); 
        
        // 进度条框
        int barW = 100;
        int barH = 8;
        int barX = (128 - barW) / 2;
        int barY = BaseY + 18;
        
        display.drawFrame(barX, barY, barW, barH);
        
        // 填充进度
        if (holdProgress > 0) {
            int fillW = (int)(holdProgress * (barW - 2));
            display.drawBox(barX + 1, barY + 1, fillW, barH - 2);
        }
    }
}