#include "apps/weather/WeatherApp.h"
#include "assets/AppIcons.h"
#include "assets/Lang.h"
#include "controller/AppController.h"
#include <cstdio>

// 引用外部硬件对象
extern DisplayHAL display;
extern InputHAL btnUp;
extern InputHAL btnDown;
extern InputHAL btnSelect;

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

// App 生命周期接口实现
void WeatherApp::onRun(AppController* sys) {
    // 初始化状态
    this->sys = sys;
    this->isExiting = false;
    ignoreClickUntil = 0;
    lastFrameTime = millis();
    viewState = VIEW_MAIN;
    slideX = 0; 
    targetSlideX = 0;
    scrollY = 0;
    targetY = 0;
    selectedIndex = 0;
    selectionSmooth = 0.0f; 
    pendingWeatherUpdate = false; 

    // 加载槽位数据
    loadSlots();
    // 缓存优先策略
    bool needImmediateRefresh = true;

    // 如果当前有选中的有效城市
    if (!slots[activeSlotIndex].isEmpty) {
        WeatherForecast cachedData; 
        // 尝试从 Flash 读取名为 "w_cache" 的缓存结构体
        bool hasCache = sys->storage.loadStruct("w_cache", cachedData);
        
        if (hasCache) {
            // 检查缓存的城市代码是否和当前选中一致
            if (strcmp(cachedData.cityCode, slots[activeSlotIndex].code) == 0) {
                // 加载缓存数据到内存
                this->forecast = cachedData;   // 载入缓存
                this->forecast.success = true; // 确保标记为成功
                needImmediateRefresh = false;  // 暂时不需要转圈圈
                
                // 检查缓存时间戳，决定是否需要静默刷新
                long cacheTime = sys->storage.getLong("w_cache_t", 0);
                long nowTime = time(NULL);
                
                // 如果时间没同步(刚开机 < 10000) 或者 缓存确实太旧了
                if (nowTime < 10000 || (nowTime - cacheTime > 7200)) {
                    pendingWeatherUpdate = true; // 标记稍后静默刷新
                }
            }
        }
    } else {
        // 如果连城市都没选，肯定不需要刷新
        needImmediateRefresh = false;
    }

    // 立即刷新天气（如果需要的话）
    if (needImmediateRefresh && !slots[activeSlotIndex].isEmpty) {
        refreshWeather();
    }

    // --- 注册按键 ---
    // UP 单击
    btnUp.attachClick([this](){
        if (viewState == VIEW_MAIN) {
            // Main页只在 Back(0) 和 City(1) 之间切换
            if (selectedIndex > 0) selectedIndex--;
        } else {
            if (selectedIndex > -1) selectedIndex--;
        }
    });

    // UP 连发
    btnUp.attachDuringLongPress([this](){
        static unsigned long lastTrig = 0;
        // 每 150ms 触发一次
        if (millis() - lastTrig > 150) {
            if (viewState != VIEW_MAIN) { // 主页只有两项，不需要连发
                if (selectedIndex > -1) selectedIndex--;
            }
            lastTrig = millis();
        }
    });

    // DOWN 单击
    btnDown.attachClick([this](){
        if (viewState == VIEW_MAIN) {
            if (selectedIndex < 1) selectedIndex++;
        } else if (viewState == VIEW_SLOTS) {
            // 5个槽位 (0-4)
            if (selectedIndex < 4) selectedIndex++;
        } else if (viewState == VIEW_LIBRARY) {
            if (selectedIndex < (int)PRESETS.size() - 1) selectedIndex++;
        }
    });

    // DOWN 连发
    btnDown.attachDuringLongPress([this](){
        static unsigned long lastTrig = 0;
        if (millis() - lastTrig > 150) {
            if (viewState == VIEW_SLOTS) {
                if (selectedIndex < 4) selectedIndex++;
            } 
            else if (viewState == VIEW_LIBRARY) {
                if (selectedIndex < (int)PRESETS.size() - 1) selectedIndex++;
            }
            lastTrig = millis();
        }
    });

    // SELECT 单击
    btnSelect.attachClick([this](){
        handleInput();
    });

    // SELECT 长按
    btnSelect.attachLongPress([this, sys](){
        if (!forecast.success && !isLoading && sys->network.isConnected()) {
            static unsigned long lastRetryTick = 0;
            if (millis() - lastRetryTick > 2000) {
                lastRetryTick = millis();
                refreshWeather(); // 只有在失败且有网时，才会自动重试
                return;
            }
        }
        if (viewState == VIEW_SLOTS) {
            // 留空，onLoop 接管
        } 
        else if (viewState == VIEW_LIBRARY) {
            viewState = VIEW_SLOTS;
            targetSlideX = -128; 
            selectedIndex = 0; 
        } 
        else {
            saveSlots();
            this->isExiting = true; 
        }
    });
}

void WeatherApp::onExit() {
    saveSlots();
}

int WeatherApp::onLoop() {
    if (this->isExiting) return 1;

    // 计算时间差
    unsigned long now = millis();                                
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;

    // 光标平滑动画
    float diffIdx = (float)selectedIndex - selectionSmooth;
    if (abs(diffIdx) > 0.01f) selectionSmooth += diffIdx * 0.25f;
    else selectionSmooth = (float)selectedIndex;

    // 视口滚动动画
    float cursorPixelY = selectionSmooth * 16.0f;
    float targetCamY = cursorPixelY - 24.0f;
    if (targetCamY < 0) targetCamY = 0;
    float diffScroll = targetCamY - scrollY;
    scrollY += diffScroll * 0.2f;

    // 长按删除逻辑 (仅在 SLOTS 视图)
    if (viewState == VIEW_SLOTS && selectedIndex >= 0 && !slots[selectedIndex].isEmpty) {
        bool isLocked = (selectedIndex == activeSlotIndex);
        if (!isLocked) {
            if (btnSelect.isPressed()) {
                deleteProgress += 0.7f * dt; 
                if (deleteProgress > 1.0f) deleteProgress = 1.0f;
                if (deleteProgress >= 1.0f) {
                    deleteCurrentSlot();
                    deleteProgress = 0.0f; 
                    ignoreClickUntil = millis() + 500; // 防误触
                }
            } else {
                deleteProgress -= 2.0f * dt;    // 松手回退
                if (deleteProgress < 0.0f) deleteProgress = 0.0f;
            }
        } else {
            deleteProgress = 0.0f;
        }
    } else {
        deleteProgress = 0.0f;
    }

    // 视图切换动画 & 静默刷新触发
    if (pendingWeatherUpdate && abs(slideX - targetSlideX) < 2.0f) {
        pendingWeatherUpdate = false;
        refreshWeather();
    }
    float diffX = targetSlideX - slideX;
    slideX += diffX * 0.2;

    render();
    return 0; 
}

void WeatherApp::refreshWeather() {
    if (slots[activeSlotIndex].isEmpty) return;
    if (!forecast.success) {
        isLoading = true;
        render(); // 立即重绘
    }
    // 发起网络请求
    WeatherForecast newForecast = sys->network.fetchForecast(WEATHER_KEY, slots[activeSlotIndex].code);
    if (newForecast.success) {
        this->forecast = newForecast;
        // 保存缓存
        strcpy(this->forecast.cityCode, slots[activeSlotIndex].code);
        sys->storage.saveStruct("w_cache", this->forecast);
        sys->storage.putLong("w_cache_t", (long)time(NULL)); // 记录时间戳
        strcpy(AppData.runtimeCache.currentCityCode, slots[activeSlotIndex].code);
        sys->forceWeatherUpdate();
    }
    isLoading = false;
    render();
}

void WeatherApp::loadSlots() {
    bool success = sys->storage.loadStruct("city_conf", slots);
    bool allEmpty = true;
    // 检查数据有效性
    for(int i=0; i<5; i++) {
        if(!slots[i].isEmpty) {
            if (strlen(slots[i].name) == 0) slots[i].isEmpty = true; 
            else allEmpty = false;
        }
    }
    if (!success || allEmpty) {
        int L = AppData.systemConfig.languageIndex;
        if (WEATHER_CITY_default < (int)PRESETS.size()) {
            strncpy(slots[0].name, PRESETS[WEATHER_CITY_default].names[L], 15);
            slots[0].name[15] = '\0';
            strcpy(slots[0].code, PRESETS[WEATHER_CITY_default].code);
            slots[0].isEmpty = false;
            for(int i=1; i<5; i++) slots[i].isEmpty = true;
        }
        // 初始化完立刻保存
        saveSlots();
        strcpy(AppData.runtimeCache.currentCityCode, slots[0].code);
    }

    activeSlotIndex = -1; 
    if (strlen(AppData.runtimeCache.currentCityCode) > 0) {
        for (int i=0; i<5; i++) {
            // 找到了匹配的城市
            if (!slots[i].isEmpty && strcmp(slots[i].code, AppData.runtimeCache.currentCityCode) == 0) {
                activeSlotIndex = i;
                break;
            }
        }
    }
    if (activeSlotIndex == -1) {
        // 遍历寻找第一个有效的城市
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
    // 更新全局当前城市代码
    if (activeSlotIndex >= 0 && !slots[activeSlotIndex].isEmpty) {
        strcpy(AppData.runtimeCache.currentCityCode, slots[activeSlotIndex].code);
    }
    // 保存槽位配置
    sys->storage.saveStruct("city_conf", slots);
    sys->storage.save(); 
}

void WeatherApp::handleInput() {
    if (millis() < ignoreClickUntil) return;
    int L = AppData.systemConfig.languageIndex;
    switch (viewState) {
        case VIEW_MAIN:
            if (selectedIndex == 0) {
                // 点击 "< Back" 退出
                this->isExiting = true;
            } else {
                // 点击城市名，进入槽位管理
                viewState = VIEW_SLOTS;
                targetSlideX = -128; 
                selectedIndex = activeSlotIndex; 
            }
            break;

        case VIEW_SLOTS:
            // 如果选中的是顶部的返回键 (-1)
            if (selectedIndex == -1) {
                viewState = VIEW_MAIN;
                targetSlideX = 0;
                selectedIndex = 1; // 回到主页选中城市名
                return;
            }

            // 选中某个槽位
            if (slots[selectedIndex].isEmpty) {
                // 点击空槽位：记录我们要编辑这个槽，但不要改变当前正在运行的天气城市
                editingSlotIndex = selectedIndex; 
                viewState = VIEW_LIBRARY;
                targetSlideX = -256; 
                selectedIndex = 0; 
            } else {
                // 切换了主城市
                if (activeSlotIndex != selectedIndex) {
                    activeSlotIndex = selectedIndex;
                    strcpy(AppData.runtimeCache.currentCityCode, slots[activeSlotIndex].code);
                    pendingWeatherUpdate = true; // 城市变了，需要刷新
                }
                viewState = VIEW_MAIN;
                targetSlideX = 0;
                selectedIndex = 1;
            }
            break;

        case VIEW_LIBRARY:
            // 处理库列表的返回键
            if (selectedIndex == -1) {
                viewState = VIEW_SLOTS;
                targetSlideX = -128;
                // 返回时，光标回到刚才想编辑的那个位置，而不是 active 位置
                selectedIndex = editingSlotIndex; 
                return;
            }

            // 选中城市，填入 editingSlotIndex 指向的槽位
            if (editingSlotIndex >= 0 && editingSlotIndex < 5) {
                CitySlot& slot = slots[editingSlotIndex];
                strncpy(slot.name, PRESETS[selectedIndex].names[L], 15);
                slot.name[15] = '\0';
                strcpy(slot.code, PRESETS[selectedIndex].code);
                slot.isEmpty = false;
                
                // 返回槽位页
                viewState = VIEW_SLOTS;
                targetSlideX = -128;
                // 光标停留在刚添加好的这个槽位上，方便用户确认
                selectedIndex = editingSlotIndex;
                
                saveSlots(); 
            }
            break;
    }
}

void WeatherApp::deleteCurrentSlot() {
    // 清除数据
    memset(&slots[selectedIndex], 0, sizeof(CitySlot));
    slots[selectedIndex].isEmpty = true;
    
    // 智能交接逻辑
    // 如果删除的正是当前主页显示的城市 (activeSlotIndex)
    if (selectedIndex == activeSlotIndex) {
        int newActive = -1;
        // 遍历所有槽位，寻找第一个有数据的槽位做替补
        for (int i = 0; i < 5; i++) {
            if (!slots[i].isEmpty) {
                newActive = i;
                break; // 找到一个就停
            }
        }
        // 更新活跃索引
        activeSlotIndex = newActive;
        // 如果找到了新城市，标记需要刷新天气
        if (newActive != -1) {
            pendingWeatherUpdate = true; 
        }
    }
    // 保存
    saveSlots();
}

int WeatherApp::getCityCount() {
    int count = 0;
    for (int i = 0; i < 5; i++) {
        if (!slots[i].isEmpty) {
            count++;
        }
    }
    return count;
}

// 将天气代码转化为对应图标和文字
const uint8_t* WeatherApp::getWeatherIcon(int code) {
    if (code == 1 || code == 3) return icon_weather_sunny_evening;  // 晚上
    if (code == 0 || code == 2) return icon_weather_sunny;          // 白天
    if (code >= 4 && code <= 9) return icon_weather_cloudy;         // 多云/阴
    if (code >= 10 && code <= 19) return icon_weather_rain;         // 雨
    if (code >= 20 && code <= 25) return icon_weather_snow;         // 雪
    if (code >= 26 && code <= 38) return icon_weather_fog;          // 雾/霾/沙
    return icon_fault;                                              // 故障
}

const char* WeatherApp::getWeatherText(int code) {
    int L = AppData.systemConfig.languageIndex; 

    switch(code) {
        // --- 晴 ---
        case 0: // 晴
        case 1: // 晴(夜)
        case 2: // 晴(大部)
        case 3: // 晴(大部夜)
            return STR_W_SUNNY[L];

        // --- 云 ---
        case 4: // 多云
        case 5: // 晴间多云
        case 6: // 晴间多云(夜)
        case 7: // 大部多云
        case 8: // 大部多云(夜)
            return STR_W_CLOUDY[L];
        case 9: // 阴
            return STR_W_OVERCAST[L];

        // --- 雨 ---
        case 10: // 阵雨
            return STR_W_SHOWER[L];
        case 11: // 雷阵雨
        case 12: // 雷阵雨伴有冰雹
            return STR_W_THUNDER[L];
        case 13: // 小雨
            return STR_W_RAIN_S[L];
        case 14: // 中雨
            return STR_W_RAIN_M[L];
        case 15: // 大雨
            return STR_W_RAIN_L[L];
        case 16: // 暴雨
        case 17: // 大暴雨
        case 18: // 特大暴雨
            return STR_W_RAIN_XL[L];
        case 19: // 冻雨
            return STR_W_RAIN_L[L]; 

        // --- 雪 ---
        case 20: // 雨夹雪
            return STR_W_SLEET[L];
        case 21: // 阵雪
        case 22: // 小雪
            return STR_W_SNOW_S[L];
        case 23: // 中雪
            return STR_W_SNOW_M[L];
        case 24: // 大雪
        case 25: // 暴雪
            return STR_W_SNOW_L[L];

        // --- 雾/霾/沙 ---
        case 30: // 雾
            return STR_W_FOG[L];
        case 31: // 霾
            return STR_W_HAZE[L];
        case 26: // 浮尘
        case 27: // 扬沙
        case 28: // 沙尘暴
        case 29: // 强沙尘暴
            return STR_W_SAND[L];

        default: 
            return STR_W_UNKNOWN[L];
    }
}

const char* WeatherApp::getSlotName(int idx) {
    if (slots[idx].isEmpty) return "";
    int L = AppData.systemConfig.languageIndex; // 获取当前系统语言
    // 1. 遍历预设列表，尝试匹配 code
    for (const auto& preset : PRESETS) {
        // 如果拼音对上了 (比如都是 "beijing")
        if (strcmp(slots[idx].code, preset.code) == 0) {
            // 返回预设列表里的名字 (这里面包含了双语动态切换)
            return preset.names[L];
        }
    }
    
    // 2. 如果预设里没找到 (比如未来支持自定义城市)，就这就用槽位里存的老名字兜底
    return slots[idx].name;
}

// --- 渲染逻辑 ---
void WeatherApp::render() {
    display.clear();
    
    int offsetX = (int)slideX;
    
    // 简单的视锥剔除优化
    if (offsetX > -128) renderMainView();
    if (offsetX < 0 && offsetX > -256) {
        display.setDrawColor(1);
        renderSlotsView();
    }
    if (offsetX < -128) renderLibraryView();

    display.update();
}

void WeatherApp::renderMainView() {
    int x = (int)slideX;
    int L = AppData.systemConfig.languageIndex; 
    char buf[32];
    
    // 确保字体背景透明，防止遮挡背景框
    display.setFontMode(1); 

    // --- Header (0px - 14px) ---
    // 绘制 Back 按钮
    if (selectedIndex == 0 && viewState == VIEW_MAIN) {
        // 选中状态：白底黑字
        display.setDrawColor(1);
        display.drawBox(x, 0, 40, 14); // 画白底
        display.setDrawColor(0);       // 设为黑笔
    } else {
        // 普通状态：黑底白字
        display.setDrawColor(1);       // 设为白笔
    }
    display.drawText(x + 2, 11, STR_BACK[L]); 

    // 绘制城市名
    display.setDrawColor(1); // 恢复白笔
    if (selectedIndex == 1 && viewState == VIEW_MAIN) {
        display.drawFrame(x + 45, 0, 83, 14); // 选中时画个空心框
    }
    
    // 城市名截断处理
    strncpy(buf, getSlotName(activeSlotIndex), 10);
    buf[10] = '\0';
    display.drawText(x + 50, 11, buf);
    
    // 分割线
    display.drawLine(x, 15, x + 128, 15);

    // --- Loading / Error 检查 ---
    if (isLoading) {
        display.drawText(x + 40, 40, STR_LOADING[L]); return;
    }
    if (!forecast.success) {
        display.drawText(x + 10, 30, STR_NET_ERR[L]); return;
    }

    // --- 列表内容 (3行等高) ---
    int startY = 17;
    int rowH = 16; 

    int tildeX = x + 44;        // 波浪号中心
    int weatherStartX = x + 68; // 天气文字起点

    for (int i = 0; i < 3; i++) {
        int rowBaseY = startY + (i * rowH);
        int textY = rowBaseY + 11;
        
        const char* labelStr = (i==0)? STR_TODAY[L] : (i==1)? STR_TOMORROW[L] : STR_DAY_AFTER[L];
        const char* weatherStr = getWeatherText(forecast.days[i].code);
        const uint8_t* iconPtr = getWeatherIcon(forecast.days[i].code);

        char lowStr[8], highStr[8];
        snprintf(lowStr, sizeof(lowStr), "%d", forecast.days[i].low);
        snprintf(highStr, sizeof(highStr), "%d", forecast.days[i].high);

        // --- 绘制 ---   
        // Col 1: 时间 (左对齐)
        display.drawText(x + 2, textY, labelStr);

        // Col 2: 温度 (以 x+48 为中心锚点)
        int tildeW = display.getStrWidth("~"); // 约 6-8px
        int lowW   = display.getStrWidth(lowStr);
        
        // 绘制低温
        display.drawText(tildeX - lowW -1, textY, lowStr);
        // 绘制 ~
        display.drawText(tildeX, textY, "~");
        // 绘制高温
        display.drawText(tildeX + tildeW + 1, textY, highStr);

        // Col 3: 天气文字 (固定左起点 x+70)
        display.drawText(weatherStartX, textY, weatherStr);

        // Col 4: 图标 (固定 x+112)
        display.setDrawColor(1);
        display.drawIcon(x + 112, rowBaseY - 1, 16, 16, iconPtr);
    }
}

void WeatherApp::renderSlotsView() {
    int baseX = (int)slideX + 128;
    int L = AppData.systemConfig.languageIndex;
    char buf[64];
    
    // 布局参数
    int listY = 28;         // 列表起始 Y
    int cursorH = 13;       // 光标高度
    int cursorOff = 10;     // 光标偏移
    int splitX = 88;        // 分割线 X
    int listWidth = 86;     // 列表宽
    int rightCenter = 108;  // 右侧中心 X

    //  --- 左侧列表绘制 ---
    if (selectedIndex >= 0) {
        int cursorDrawY = listY + (int)(selectionSmooth * 16) - (int)scrollY - cursorOff;
        if (cursorDrawY > 14 && cursorDrawY < 64) {
            display.setDrawColor(1);
            display.drawBox(baseX, cursorDrawY, listWidth, cursorH);
        }
    }

    for (int i = 0; i < 5; i++) {
        int drawY = listY + (i * 16) - (int)scrollY;
        if (drawY > 70 || drawY < 10) continue;

        if (i == selectedIndex && selectedIndex >= 0) display.setDrawColor(0);
        else display.setDrawColor(1);

        if (slots[i].isEmpty) {
            display.drawText(baseX + 4, drawY, STR_EMPTY_SLOT[L]); 
        } else {
            const char* dynamicName = getSlotName(i);
            if (i == activeSlotIndex) snprintf(buf, sizeof(buf), "* %s", dynamicName);
            else snprintf(buf, sizeof(buf), "  %s", dynamicName);
            display.drawText(baseX + 4, drawY, buf);
        }
    }

    // --- 右侧区域绘制 ---
    display.setDrawColor(1);
    display.drawLine(baseX + splitX, 15, baseX + splitX, 64); // 分割线

    // 只有在选中有效槽位时才画右侧内容
    if (selectedIndex >= 0 && selectedIndex < 5) {
        int midY = 42; 
        
        // 情况 A: 这是一个空槽位 -> 显示添加
        if (slots[selectedIndex].isEmpty) {
            // 画一个加号
            display.drawLine(baseX + rightCenter - 5, midY, baseX + rightCenter + 5, midY);
            display.drawLine(baseX + rightCenter, midY - 5, baseX + rightCenter, midY + 5);
            display.drawText(baseX + splitX + 8, 57, STR_ADD[L]);
        } 
        // 情况 B: 这是一个已有城市
        else {
            bool isLocked = (selectedIndex == activeSlotIndex);
            if (isLocked) {
                // 只有这一个独苗 -> 显示锁 (禁止删除) ---
                // 画一个简单的锁头
                display.drawFrame(baseX + rightCenter - 4, midY - 2, 8, 6); // 锁身
                display.drawBox(baseX + rightCenter - 4, midY - 2, 8, 6);   // 实心锁身
                display.drawLine(baseX + rightCenter - 2, midY - 2, baseX + rightCenter - 2, midY - 5); // 锁梁左
                display.drawLine(baseX + rightCenter + 2, midY - 2, baseX + rightCenter + 2, midY - 5); // 锁梁右
                display.drawLine(baseX + rightCenter - 2, midY - 5, baseX + rightCenter + 2, midY - 5); // 锁梁顶
                display.drawText(baseX + splitX + 10, 57, STR_LOCK[L]);
            } 
            else {
                // 可以删除 -> 显示进度条和提示
                // 1. 文字提示 (HOLD / DEL)
                // Y=38 左右显示文字
                if (deleteProgress > 0) {
                    display.drawText(baseX + splitX + 8, 38, STR_DEL[L]);
                } else {
                    display.drawText(baseX + splitX + 6, 38, STR_HOLD[L]);
                }

                // 2. 进度条 (横向更稳妥，或者保持纵向但下移)
                // 这里我们把进度条画在文字下方 (Y=42 到 Y=60)
                int barW = 6;
                int barH = 18; // 稍微短一点
                int barX = baseX + rightCenter - (barW/2);
                int barY = 44; // 在文字下方
                
                display.drawFrame(barX, barY, barW, barH);
                
                if (deleteProgress > 0.01f) {
                    // 从下往上填
                    int fillH = (int)(deleteProgress * barH);
                    display.drawBox(barX, barY + barH - fillH, barW, fillH);
                }
            }
        }
    }
    // 情况 C: 返回键
    else if (selectedIndex == -1) {
        display.drawText(baseX + splitX + 10, 40, STR_EXIT[L]);
    }

    // --- 3. 标题栏 (保持不变) ---
    display.setDrawColor(0); 
    display.drawBox(baseX, 0, 128, 15); 
    display.setDrawColor(1);
    display.drawText(baseX + 2, 12, STR_CITY_SELECT[L]);
    int backBtnX = baseX + 90;
    if (selectedIndex == -1) {
        display.drawBox(backBtnX - 2, 0, 38, 14);
        display.setDrawColor(0);
    } else {
        display.setDrawColor(1);
    }
    display.drawText(backBtnX, 12, STR_BACK[L]);
    display.setDrawColor(1); 
    display.drawLine(baseX, 14, baseX+128, 14);
}

void WeatherApp::renderLibraryView() {
    int baseX = (int)slideX + 256;
    int L = AppData.systemConfig.languageIndex;
    int itemH = 16; 
    int listY = 28;
    int cursorBoxH = 13; 
    int cursorOff = 10;  

    // 绘制列表光标 (selectedIndex >= 0)
    if (selectedIndex >= 0) {
        // 计算平滑光标位置
        int cursorDrawY = listY + (int)(selectionSmooth * 16) - (int)scrollY - cursorOff;
        // 仅当光标在可见区域时才绘制
        if (cursorDrawY > 14 && cursorDrawY < 64) {
            display.setDrawColor(1);
            display.drawBox(baseX, cursorDrawY, 100, cursorBoxH);
        }
    }
    // 绘制列表项
    for (int i = 0; i < (int)PRESETS.size(); i++) {
        int drawY = listY + (i * itemH) - (int)scrollY;
        if (drawY > 64) continue; 
        if (drawY < 14) continue; 

        // 绘制文字
        if (i == selectedIndex && selectedIndex >= 0 && viewState == VIEW_LIBRARY) {
            display.setDrawColor(0);
        } else {
            display.setDrawColor(1);
        }
        display.drawText(baseX + 4, drawY, PRESETS[i].names[L]);
    }

    // 标题栏背景
    display.setDrawColor(0); 
    display.drawBox(baseX, 0, 128, 15);
    display.setDrawColor(1);
    display.drawText(baseX + 2, 12, STR_ADD_CITY[L]); 
    
    // 右上角返回键
    int backBtnX = baseX + 90;

    if (selectedIndex == -1) {
        display.drawBox(backBtnX - 2, 0, 40, 14);
        display.setDrawColor(0);
    } else {
        display.setDrawColor(1);
    }
    display.drawText(backBtnX, 12, STR_BACK[L]);

    display.setDrawColor(1);
    display.drawLine(baseX, 14, baseX+128, 14);
}