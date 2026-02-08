#include "apps/weather/WeatherApp.h"
#include "assets/AppIcons.h"
#include "assets/Lang.h"
#include "controller/AppController.h"
#include <cstdio>
#include <cmath>
#include <cstring>

// 预设城市列表
const std::vector<PresetCity> WeatherApp::PRESETS = {
    {STR_Hefei,     "hefei"},
    {STR_Beijing,   "beijing"},
    {STR_Shanghai,  "shanghai"},
    {STR_Guangzhou, "guangzhou"},
    {STR_Shenzhen,  "shenzhen"},
    {STR_Chengdu,   "chengdu"},
    {STR_Hangzhou,  "hangzhou"},
    {STR_Wuhan,     "wuhan"},
    {STR_Xian,      "xian"},
    {STR_Nanjing,   "nanjing"},
    {STR_Suzhou,    "suzhou"}
};

// -- 辅助函数 --------------------------------------------------------------------

void WeatherApp::loadSlots() {
    bool success = sys->storage.loadStruct("city_conf", slots);
    bool allEmpty = true;
    for (int i = 0; i < 5; i++) {
        if (!slots[i].isEmpty) {
            if (strlen(slots[i].name) == 0) slots[i].isEmpty = true; 
            else allEmpty = false;
        }
    }
    if (!success || allEmpty) {
        int L = AppData.systemConfig.languageIndex;
        if (!PRESETS.empty()) {
            strncpy(slots[0].name, PRESETS[0].names[L], 15);
            slots[0].name[15] = '\0';
            strcpy(slots[0].code, PRESETS[0].code);
            slots[0].isEmpty = false;
            for(int i=1; i<5; i++) slots[i].isEmpty = true;
        }
        saveSlots();
        strcpy(AppData.runtimeCache.currentCityCode, slots[0].code);
    }
    activeSlotIndex = -1; 
    if (strlen(AppData.runtimeCache.currentCityCode) > 0) {
        for (int i=0; i<5; i++) {
            if (!slots[i].isEmpty && strcmp(slots[i].code, AppData.runtimeCache.currentCityCode) == 0) {
                activeSlotIndex = i;
                break;
            }
        }
    }
    if (activeSlotIndex == -1) {
        for (int i = 0; i < 5; i++) {
            if (!slots[i].isEmpty) {
                activeSlotIndex = i;
                strcpy(AppData.runtimeCache.currentCityCode, slots[i].code);
                break;
            }
        }
    }
    if (activeSlotIndex == -1) activeSlotIndex = 0;
}

void WeatherApp::saveSlots() {
    if (activeSlotIndex >= 0 && !slots[activeSlotIndex].isEmpty) {
        strcpy(AppData.runtimeCache.currentCityCode, slots[activeSlotIndex].code);
    }
    sys->storage.saveStruct("city_conf", slots);
    sys->storage.save(); 
}

void WeatherApp::refreshWeather() {
    if (slots[activeSlotIndex].isEmpty) return;
    if (!forecast.success) { isLoading = true; render(); }
    
    WeatherForecast newForecast = sys->network.fetchForecast(WEATHER_KEY, slots[activeSlotIndex].code);
    if (newForecast.success) {
        this->forecast = newForecast;
        strcpy(this->forecast.cityCode, slots[activeSlotIndex].code);
        sys->storage.saveStruct("w_cache", this->forecast);
        sys->storage.putLong("w_cache_t", (long)time(NULL));
        strcpy(AppData.runtimeCache.currentCityCode, slots[activeSlotIndex].code);
        sys->forceWeatherUpdate();
    }
    isLoading = false;
    render();
}

void WeatherApp::deleteCurrentSlot() {
    memset(&slots[selectedIndex], 0, sizeof(CitySlot));
    slots[selectedIndex].isEmpty = true;
    if (selectedIndex == activeSlotIndex) {
        int newActive = -1;
        for (int i = 0; i < 5; i++) {
            if (!slots[i].isEmpty) { newActive = i; break; }
        }
        activeSlotIndex = newActive;
        if (newActive != -1) pendingWeatherUpdate = true; 
    }
    saveSlots();
}

const char* WeatherApp::getSlotName(int idx) {
    if (slots[idx].isEmpty) return "";
    int L = AppData.systemConfig.languageIndex;
    for (const auto& preset : PRESETS) {
        if (strcmp(slots[idx].code, preset.code) == 0) return preset.names[L];
    }
    return slots[idx].name;
}

const uint8_t* WeatherApp::getWeatherIcon(int code) {
    if (code == 1 || code == 3) return icon_weather_sunny_evening;
    if (code == 0 || code == 2) return icon_weather_sunny;
    if (code >= 4 && code <= 9) return icon_weather_cloudy;
    if (code >= 10 && code <= 19) return icon_weather_rain;
    if (code >= 20 && code <= 25) return icon_weather_snow;
    if (code >= 26 && code <= 38) return icon_weather_fog;
    return icon_fault;
}

const char* WeatherApp::getWeatherText(int code) {
    int L = AppData.systemConfig.languageIndex; 
    switch(code) {
        case 0: case 1: case 2: case 3: return STR_W_SUNNY[L];
        case 4: case 5: case 6: case 7: case 8: return STR_W_CLOUDY[L];
        case 9: return STR_W_OVERCAST[L];
        case 10: return STR_W_SHOWER[L];
        case 11: case 12: return STR_W_THUNDER[L];
        case 13: return STR_W_RAIN_S[L];
        case 14: return STR_W_RAIN_M[L];
        case 15: case 19: return STR_W_RAIN_L[L];
        case 16: case 17: case 18: return STR_W_RAIN_XL[L];
        case 20: return STR_W_SLEET[L];
        case 21: case 22: return STR_W_SNOW_S[L];
        case 23: return STR_W_SNOW_M[L];
        case 24: case 25: return STR_W_SNOW_L[L];
        case 30: return STR_W_FOG[L];
        case 31: return STR_W_HAZE[L];
        case 26: case 27: case 28: case 29: return STR_W_SAND[L];
        default: return STR_W_UNKNOWN[L];
    }
}

// -- 按键处理 --------------------------------------------------------------------

void WeatherApp::onKeyUp() {
    if (viewState == VIEW_MAIN) {
        if (selectedIndex > 0) selectedIndex--;
    } else {
        if (selectedIndex > -1) selectedIndex--;
    }
}

void WeatherApp::onKeyDown() {
    if (viewState == VIEW_MAIN) {
        if (selectedIndex < 1) selectedIndex++;
    } else if (viewState == VIEW_SLOTS) {
        if (selectedIndex < 4) selectedIndex++;
    } else if (viewState == VIEW_LIBRARY) {
        if (selectedIndex < (int)PRESETS.size() - 1) selectedIndex++;
    }
}

void WeatherApp::onKeySelect() {
    if (millis() < ignoreClickUntil) return;
    int L = AppData.systemConfig.languageIndex;

    switch (viewState) {
        case VIEW_MAIN:
            if (selectedIndex == 0) this->isExiting = true;
            else {
                viewState = VIEW_SLOTS;
                targetSlideX = -128; 
                selectedIndex = activeSlotIndex; 
            }
            break;
        case VIEW_SLOTS:
            if (selectedIndex == -1) {
                viewState = VIEW_MAIN;
                targetSlideX = 0;
                selectedIndex = 1; 
                return;
            }
            if (slots[selectedIndex].isEmpty) {
                editingSlotIndex = selectedIndex; 
                viewState = VIEW_LIBRARY;
                targetSlideX = -256; 
                selectedIndex = 0; 
            } else {
                if (activeSlotIndex != selectedIndex) {
                    activeSlotIndex = selectedIndex;
                    strcpy(AppData.runtimeCache.currentCityCode, slots[activeSlotIndex].code);
                    pendingWeatherUpdate = true;
                }
                viewState = VIEW_MAIN;
                targetSlideX = 0;
                selectedIndex = 1;
            }
            break;
        case VIEW_LIBRARY:
            if (selectedIndex == -1) {
                viewState = VIEW_SLOTS;
                targetSlideX = -128;
                selectedIndex = editingSlotIndex; 
                return;
            }
            if (editingSlotIndex >= 0 && editingSlotIndex < 5) {
                CitySlot& slot = slots[editingSlotIndex];
                strncpy(slot.name, PRESETS[selectedIndex].names[L], 15);
                slot.name[15] = '\0';
                strcpy(slot.code, PRESETS[selectedIndex].code);
                slot.isEmpty = false;
                viewState = VIEW_SLOTS;
                targetSlideX = -128;
                selectedIndex = editingSlotIndex;
                saveSlots(); 
            }
            break;
    }
}

void WeatherApp::onKeyLongPressSelect() {
    if (!forecast.success && !isLoading && sys->network.isConnected()) {
        static unsigned long lastRetryTick = 0;
        if (millis() - lastRetryTick > 2000) {
            lastRetryTick = millis();
            refreshWeather();
            return;
        }
    }
    if (viewState == VIEW_SLOTS) {
        // 槽位页长按由 Hold 逻辑处理
    } else if (viewState == VIEW_LIBRARY) {
        viewState = VIEW_SLOTS;
        targetSlideX = -128; 
        selectedIndex = 0; 
    } else {
        saveSlots();
        this->isExiting = true; 
    }
}

void WeatherApp::onKeyHoldSelect(float dt) {
    if (viewState == VIEW_SLOTS && selectedIndex >= 0 && !slots[selectedIndex].isEmpty) {
        bool isLocked = (selectedIndex == activeSlotIndex);
        if (!isLocked) {
            if (sys->btnSelect.isPressed()) {
                deleteProgress += 0.7f * dt; 
                if (deleteProgress > 1.0f) deleteProgress = 1.0f;
                if (deleteProgress >= 1.0f) {
                    deleteCurrentSlot();
                    deleteProgress = 0.0f; 
                    ignoreClickUntil = millis() + 500;
                    if (!sys->processInput()) return; 
                }
            } else {
                deleteProgress -= 2.0f * dt;
                if (deleteProgress < 0.0f) deleteProgress = 0.0f;
            }
        } else deleteProgress = 0.0f;
    } else deleteProgress = 0.0f;
}

// -- 生命周期 --------------------------------------------------------------------

void WeatherApp::onRun(AppController* sys) {
    this->sys = sys;
    this->isExiting = false;
    ignoreClickUntil = 0;
    lastFrameTime = millis();
    viewState = VIEW_MAIN;
    slideX = 0; 
    targetSlideX = 0;
    scrollY = 0;
    selectedIndex = 0;
    selectionSmooth = 0.0f; 
    pendingWeatherUpdate = false; 

    loadSlots();
    
    // 自动刷新逻辑
    bool needImmediateRefresh = true;
    if (!slots[activeSlotIndex].isEmpty) {
        WeatherForecast cachedData; 
        bool hasCache = sys->storage.loadStruct("w_cache", cachedData);
        if (hasCache && strcmp(cachedData.cityCode, slots[activeSlotIndex].code) == 0) {
            this->forecast = cachedData;
            this->forecast.success = true;
            needImmediateRefresh = false;
            long cacheTime = sys->storage.getLong("w_cache_t", 0);
            long nowTime = time(NULL);
            if (nowTime < 10000 || (nowTime - cacheTime > 7200)) pendingWeatherUpdate = true;
        }
    } else needImmediateRefresh = false;

    if (needImmediateRefresh && !slots[activeSlotIndex].isEmpty) refreshWeather();

    sys->btnUp.attachClick([this, sys]() { if (!sys->processInput()) return; this->onKeyUp(); });
    sys->btnDown.attachClick([this, sys]() { if (!sys->processInput()) return; this->onKeyDown(); });
    sys->btnSelect.attachClick([this, sys]() { if (!sys->processInput()) return; this->onKeySelect(); });
    sys->btnSelect.attachLongPress([this, sys]() { if (!sys->processInput()) return; this->onKeyLongPressSelect(); });

    sys->btnUp.attachDuringLongPress([this, sys]() {
        if (!sys->processInput()) return;
        static unsigned long lastTrig = 0;
        if (millis() - lastTrig > 150) { this->onKeyUp(); lastTrig = millis(); }
    });
    sys->btnDown.attachDuringLongPress([this, sys]() {
        if (!sys->processInput()) return;
        static unsigned long lastTrig = 0;
        if (millis() - lastTrig > 150) { this->onKeyDown(); lastTrig = millis(); }
    });
}

int WeatherApp::onLoop() {
    if (this->isExiting) return 1;

    unsigned long now = millis();                                
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;

    onKeyHoldSelect(dt);

    float diffIdx = (float)selectedIndex - selectionSmooth;
    if (fabs(diffIdx) > 0.01f) selectionSmooth += diffIdx * 0.25f;
    else selectionSmooth = (float)selectedIndex;

    if (viewState == VIEW_SLOTS) {
        float cursorTargetY = (selectedIndex < 0) ? 0 : selectionSmooth * LIST_ITEM_H;
        float targetCamY = cursorTargetY - 24 + 8;

        if (targetCamY < 0) targetCamY = 0;
        float maxScroll = (5 * LIST_ITEM_H) - 48 + 10; // 5个槽位
        if (maxScroll < 0) maxScroll = 0;
        if (targetCamY > maxScroll) targetCamY = maxScroll;
        
        scrollY += (targetCamY - scrollY) * 0.2f;
    } else if (viewState == VIEW_LIBRARY) {
        float cursorPixelY = selectionSmooth * 16.0f;
        float targetCamY = cursorPixelY - 24.0f;
        if (targetCamY < 0) targetCamY = 0;
        float diffScroll = targetCamY - scrollY;
        scrollY += diffScroll * 0.2f;
    } else {
        scrollY = 0;
    }

    if (pendingWeatherUpdate && fabs(slideX - targetSlideX) < 2.0f) {
        pendingWeatherUpdate = false;
        refreshWeather();
    }
    float diffX = targetSlideX - slideX;
    slideX += diffX * 0.2f;

    render();
    return 0; 
}

void WeatherApp::onExit() {
    saveSlots();
}

// -- 绘图渲染 --------------------------------------------------------------------

void WeatherApp::render() {
    sys->display.clear();
    int offsetX = (int)slideX;
    if (offsetX > -128) renderMainView();
    if (offsetX < 0 && offsetX > -256) {
        sys->display.setDrawColor(1);
        renderSlotsView();
    }
    if (offsetX < -128) renderLibraryView();
}

void WeatherApp::renderMainView() {
    int x = (int)slideX;
    int L = AppData.systemConfig.languageIndex; 
    char buf[32];
    sys->display.setFontMode(1); 

    if (selectedIndex == 0 && viewState == VIEW_MAIN) {
        sys->display.setDrawColor(1);
        sys->display.drawBox(x, 0, 40, 14);
        sys->display.setDrawColor(0);
    } else sys->display.setDrawColor(1);
    sys->display.drawText(x + 2, 11, STR_BACK[L]); 

    sys->display.setDrawColor(1);
    if (selectedIndex == 1 && viewState == VIEW_MAIN) {
        sys->display.drawFrame(x + 45, 0, 83, 14);
    }
    
    strncpy(buf, getSlotName(activeSlotIndex), 10);
    buf[10] = '\0';
    sys->display.drawText(x + 50, 11, buf);
    sys->display.drawLine(x, 15, x + 128, 15);

    if (isLoading) { sys->display.drawText(x + 40, 40, STR_LOADING[L]); return; }
    if (!forecast.success) { sys->display.drawText(x + 10, 30, STR_NET_ERR[L]); return; }

    int startY = 17;
    int rowH = 16; 
    int tildeX = x + 44;
    int weatherStartX = x + 68;

    for (int i = 0; i < 3; i++) {
        int rowBaseY = startY + (i * rowH);
        int textY = rowBaseY + 11;
        const char* labelStr = (i==0)? STR_TODAY[L] : (i==1)? STR_TOMORROW[L] : STR_DAY_AFTER[L];
        const char* weatherStr = getWeatherText(forecast.days[i].code);
        const uint8_t* iconPtr = getWeatherIcon(forecast.days[i].code);
        char lowStr[8], highStr[8];
        snprintf(lowStr, sizeof(lowStr), "%d", forecast.days[i].low);
        snprintf(highStr, sizeof(highStr), "%d", forecast.days[i].high);

        sys->display.drawText(x + 2, textY, labelStr);
        int tildeW = sys->display.getStrWidth("~");
        int lowW   = sys->display.getStrWidth(lowStr);
        sys->display.drawText(tildeX - lowW -1, textY, lowStr);
        sys->display.drawText(tildeX, textY, "~");
        sys->display.drawText(tildeX + tildeW + 1, textY, highStr);
        sys->display.drawText(weatherStartX, textY, weatherStr);
        sys->display.setDrawColor(1);
        sys->display.drawIcon(x + 112, rowBaseY - 1, 16, 16, iconPtr);
    }
}

void WeatherApp::renderSlotsView() {
    int baseX = (int)slideX + 128; // 偏移量，ViewSlots 在第二个屏
    int L = AppData.systemConfig.languageIndex;
    char buf[64];

    sys->display.setFont(u8g2_font_wqy12_t_gb2312);
    sys->display.setFontMode(1);
    sys->display.setDrawColor(1);

    // 1. 绘制基本线条
    sys->display.drawLine(baseX, HEADER_H - 1, baseX + 128, HEADER_H - 1);
    sys->display.drawLine(baseX + SPLIT_X, 0, baseX + SPLIT_X, 64);
    
    sys->display.drawText(baseX + 4, 11, STR_CITY_SELECT[L]);

    // 2. 返回按钮高亮逻辑 (selectedIndex == -1)
    if (selectedIndex == -1) {
        sys->display.drawBox(baseX + SPLIT_X + 1, 0, 128 - SPLIT_X, HEADER_H);
        sys->display.setDrawColor(0);
    }
    
    int strW = sys->display.getStrWidth(STR_BACK[L]);
    int boxCenter = SPLIT_X + (128 - SPLIT_X) / 2;
    sys->display.drawText(baseX + boxCenter - (strW / 2), 11, STR_BACK[L]);
    sys->display.setDrawColor(1);

    // 3. 设置裁剪区域，防止列表溢出到 Header 或右侧
    sys->display.setClipWindow(baseX, HEADER_H, baseX + SPLIT_X, 64);

    // 4. 绘制列表光标 (圆角矩形)
    if (selectedIndex >= 0 && selectionSmooth > -0.8f) {
        int cursorY = HEADER_H + (int)(selectionSmooth * LIST_ITEM_H) - (int)scrollY;
        // 确保光标在列表区域内才绘制
        if (cursorY > HEADER_H - LIST_ITEM_H && cursorY < 64) {
            sys->display.setDrawColor(1);
            // 宽度 = SPLIT_X - 4, 也就是列表左侧区域
            sys->display.drawRBox(baseX + 2, cursorY + 1, SPLIT_X - 4, LIST_ITEM_H - 2, 2);
        }
    }

    // 5. 绘制列表项 (0~4)
    for (int i = 0; i < 5; i++) {
        int drawY = HEADER_H + (i * LIST_ITEM_H) - (int)scrollY;
        // 剔除屏幕外的项
        if (drawY > 64 || drawY < HEADER_H - LIST_ITEM_H) continue;
        
        // 选中时反色，否则正色
        bool isSelected = (selectedIndex == i);
        sys->display.setDrawColor(isSelected ? 0 : 1);

        if (slots[i].isEmpty) {
            sys->display.drawText(baseX + 6, drawY + 11, STR_EMPTY_SLOT[L]); 
        } else {
            const char* dynamicName = getSlotName(i);
            if (i == activeSlotIndex) snprintf(buf, sizeof(buf), "* %s", dynamicName);
            else snprintf(buf, sizeof(buf), "  %s", dynamicName);
            sys->display.drawText(baseX + 6, drawY + 11, buf);
        }
    }
    sys->display.setMaxClipWindow(); // 恢复裁剪
    sys->display.setDrawColor(1);

    // 6. 右侧操作区显示 (New / Lock / Del)
    if (selectedIndex >= 0 && selectedIndex < 5) {
        int centerX = baseX + RIGHT_CENTER;
        int centerY = 40;

        if (slots[selectedIndex].isEmpty) {
            // 空槽位 -> 显示 New / Add 图标
            sys->display.drawLine(centerX - 4, centerY, centerX + 4, centerY);
            sys->display.drawLine(centerX, centerY - 4, centerX, centerY + 4);
            sys->display.drawText(centerX - 9, centerY + 16, STR_ADD[L]);
        } else {
            bool isLocked = (selectedIndex == activeSlotIndex);
            if (isLocked) {
                // 当前活动城市 -> 显示锁图标
                sys->display.drawFrame(centerX - 4, centerY - 2, 8, 6);
                sys->display.drawBox(centerX - 4, centerY - 2, 8, 6);
                sys->display.drawLine(centerX - 2, centerY - 2, centerX - 2, centerY - 5);
                sys->display.drawLine(centerX + 2, centerY - 2, centerX + 2, centerY - 5);
                sys->display.drawLine(centerX - 2, centerY - 5, centerX + 2, centerY - 5);
                sys->display.drawText(centerX - 9, centerY + 16, STR_LOCK[L]);
            } else {
                // 普通槽位 -> 显示删除进度或提示
                if (deleteProgress > 0.1f) {
                    sys->display.drawText(centerX - 9, 28, STR_DEL[L]);
                    int barW = 6, barH = 20, barX = centerX - 3, barY = 32;
                    sys->display.drawFrame(barX, barY, barW, barH);
                    int fillH = (int)(deleteProgress * barH);
                    if (fillH > barH) fillH = barH;
                    sys->display.drawBox(barX, barY + barH - fillH, barW, fillH);
                } else {
                    sys->display.drawText(centerX - 9, centerY + 16, STR_HOLD[L]);
                }
            }
        }
    } else if (selectedIndex == -1) {
        // 选中 Back 按钮时，右侧显示退出提示
        int centerX = baseX + RIGHT_CENTER;
        sys->display.drawText(centerX - 9, 40, STR_EXIT[L]);
    }
}

void WeatherApp::renderLibraryView() {
    int baseX = (int)slideX + 256;
    int L = AppData.systemConfig.languageIndex;
    int itemH = 16; 
    int listY = 28;
    int cursorBoxH = 14; 
    int cursorOff = 10;  

    if (selectedIndex >= 0) {
        int cursorDrawY = listY + (int)(selectionSmooth * 16) - (int)scrollY - cursorOff;
        if (cursorDrawY > 14 && cursorDrawY < 64) {
            sys->display.setDrawColor(1);
            sys->display.drawRBox(baseX + 2, cursorDrawY, 124, cursorBoxH, 2);
        }
    }

    for (int i = 0; i < (int)PRESETS.size(); i++) {
        int drawY = listY + (i * itemH) - (int)scrollY;
        if (drawY > 64) continue; 
        if (drawY < 14) continue; 

        if (i == selectedIndex && selectedIndex >= 0 && viewState == VIEW_LIBRARY) {
            sys->display.setDrawColor(0);
        } else {
            sys->display.setDrawColor(1);
        }
        sys->display.drawText(baseX + 6, drawY, PRESETS[i].names[L]);
    }

    sys->display.setDrawColor(0); 
    sys->display.drawBox(baseX, 0, 128, 15);
    sys->display.setDrawColor(1);
    sys->display.drawText(baseX + 2, 12, STR_ADD_CITY[L]); 
    
    int backBtnX = baseX + 90;
    if (selectedIndex == -1) {
        sys->display.drawBox(backBtnX - 2, 0, 40, 14);
        sys->display.setDrawColor(0);
    } else {
        sys->display.setDrawColor(1);
    }
    sys->display.drawText(backBtnX, 12, STR_BACK[L]);

    sys->display.setDrawColor(1);
    sys->display.drawLine(baseX, 14, baseX+128, 14);
}