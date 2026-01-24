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
    {STR_Beijing,   "beijing"},
    {STR_Shanghai,  "shanghai"},
    {STR_Guangzhou, "guangzhou"},
    {STR_Shenzhen,  "shenzhen"},
    {STR_Chengdu,   "chengdu"},
    {STR_Hangzhou,  "hangzhou"},
    {STR_Wuhan,     "wuhan"},
    {STR_Xian,      "xian"},
    {STR_Nanjing,   "nanjing"},
    {STR_Suzhou,    "suzhou"},
    {STR_Hefei,     "hefei"}
};

// App 生命周期接口实现
void WeatherApp::onRun(AppController* sys) {
    // 初始化状态
    this->sys = sys;
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

    loadSlots();
    
    // 如果当前槽位有数据，且没有天气数据，则刷新
    if (!slots[activeSlotIndex].isEmpty) {
        refreshWeather();
    }

    // --- 注册按键 ---
    btnUp.attachClick([this](){
        if (viewState == VIEW_MAIN) {
            // Main页只在 Back(0) 和 City(1) 之间切换
            if (selectedIndex > 0) selectedIndex--;
        } else {
            if (selectedIndex > -1) selectedIndex--;
        }
    });

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

    btnSelect.attachClick([this](){
        handleInput();
    });

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
            // 这里留空！
            // 我们在 onLoop 里手动接管了长按进度条和删除逻辑。
            // 如果这里再写代码，就会出现“锁住了但还能删”的 Bug。
        } 
        else if (viewState == VIEW_LIBRARY) {
            viewState = VIEW_SLOTS;
            targetSlideX = -128; 
            selectedIndex = 0; 
        } 
        else {
            saveSlots();
            sys->quitApp(); 
        }
    });
}

void WeatherApp::onExit() {
    saveSlots();
}

int WeatherApp::onLoop() {
    unsigned long now = millis();                                
    // 计算两帧之间的时间差 (秒)
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;

    // ----- 光标动画 -----
    float diffIdx = (float)selectedIndex - selectionSmooth;
    if (abs(diffIdx) > 0.01f) selectionSmooth += diffIdx * 0.25f;
    else selectionSmooth = (float)selectedIndex;

    // ----- 视口滚动 -----
    float cursorPixelY = selectionSmooth * 16.0f;
    float targetCamY = cursorPixelY - 24.0f;
    if (targetCamY < 0) targetCamY = 0;
    float diffScroll = targetCamY - scrollY;
    scrollY += diffScroll * 0.2f;

    // --- 长按删除检测逻辑 ---
    // 只有在 SLOTS 视图，且选中了有效城市
    if (viewState == VIEW_SLOTS && selectedIndex >= 0 && !slots[selectedIndex].isEmpty) {
        
        // 规则：只有当城市数量 > 1 时，才允许删除
        if (getCityCount() > 1) {
            
            if (btnSelect.isPressed()) {
                // 目标：1.5秒充满。 即 1.0 / 1.5 = 0.66 per second
                deleteProgress += 0.7f * dt; 
                // 封顶
                if (deleteProgress > 1.0f) deleteProgress = 1.0f;
                // 满了就删除
                if (deleteProgress >= 1.0f) {
                    deleteCurrentSlot();
                    deleteProgress = 0.0f; 
                    
                    // 删除后，设置 500ms 的点击冷却时间
                    // 这样用户松手时产生的 click 信号会被 handleInput 忽略
                    ignoreClickUntil = millis() + 500;
                }
            } else {
                // 松手回退 (稍微快一点回退)
                deleteProgress -= 2.0f * dt;
                if (deleteProgress < 0.0f) deleteProgress = 0.0f;
            }
        } else {
            // 只有一个城市，锁住，不准动进度
            deleteProgress = 0.0f;
        }
    } else {
        deleteProgress = 0.0f;
    }

    // --- 视图切换动画 ---
    if (pendingWeatherUpdate && abs(slideX - targetSlideX) < 2.0f) {
        pendingWeatherUpdate = false;
        refreshWeather();
    }
    float diffX = targetSlideX - slideX;
    slideX += diffX * 0.2;

    render();
    return 0; 
}

void WeatherApp::handleInput() {
    if (millis() < ignoreClickUntil) return;
    int L = AppData.languageIndex;
    switch (viewState) {
        case VIEW_MAIN:
            if (selectedIndex == 0) {
                // 点击 "< Back" 退出
                sys->quitApp();
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
                // 点击已有城市：这才是真正切换“当前天气城市”的时候
                activeSlotIndex = selectedIndex;
                viewState = VIEW_MAIN;
                targetSlideX = 0;
                selectedIndex = 1;
                pendingWeatherUpdate = true;
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
                strcpy(slot.name, PRESETS[selectedIndex].names[L]);
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

void WeatherApp::refreshWeather() {
    if (slots[activeSlotIndex].isEmpty) return;
    isLoading = true;
    render(); // 立即重绘以显示 Loading
    
    // 发起网络请求
    forecast = sys->network.fetchForecast(WEATHER_KEY, slots[activeSlotIndex].code);
    isLoading = false;
}

void WeatherApp::loadSlots() {
    int L = AppData.languageIndex;
    for (int i=0; i<5; i++) slots[i].isEmpty = true;
    strcpy(slots[0].name, PRESETS[WEATHER_CITY_index].names[L]);
    strcpy(slots[0].code, PRESETS[WEATHER_CITY_index].code);
    slots[0].isEmpty = false;
}

void WeatherApp::saveSlots() {
    // 暂未实现
}

const uint8_t* WeatherApp::getWeatherIcon(int code) {
    // 夜间晴/夜间多云
    if (code == 1 || code == 3) {
        return icon_weather_sunny_evening; 
    }

    // 白天晴
    if (code == 0 || code == 2) {
        return icon_weather_sunny;
    }
    
    // 多云 / 阴天
    if (code >= 4 && code <= 9) {
        return icon_weather_cloudy;
    }
    
    // 雨
    if (code >= 10 && code <= 19) {
        return icon_weather_rain;
    }
    
    // 雪
    if (code >= 20 && code <= 25) {
        return icon_weather_snow;
    }
    
    // 雾/霾/沙尘
    if (code >= 26 && code <= 38) {
        return icon_weather_fog;
    }

    return icon_weather_sunny; 
}

// 辅助函数：将天气代码转换为对应语言的字符串
const char* WeatherApp::getWeatherText(int code) {
    int L = AppData.languageIndex; 

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
    int L = AppData.languageIndex; 
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
    strncpy(buf, slots[activeSlotIndex].name, 10);
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

    for (int i = 0; i < 3; i++) {
        int rowBaseY = startY + (i * rowH);
        int textY = rowBaseY + 11;
        
        // --- 数据准备 ---
        const char* labelStr;
        if (i == 0) labelStr = STR_TODAY[L];
        else if (i == 1) labelStr = STR_TOMORROW[L];
        else labelStr = STR_DAY_AFTER[L];
        char tempStr[16];
        snprintf(tempStr, sizeof(tempStr), "%d~%d", forecast.days[i].low, forecast.days[i].high);

        const char* weatherStr = getWeatherText(forecast.days[i].code);
        const uint8_t* iconPtr = getWeatherIcon(forecast.days[i].code);

        // --- 绘制 ---   
        // Col 1: 时间 (Max 24px)
        // 起点 x+2
        display.drawText(x + 2, textY, labelStr);
        // Col 2: 温度 (Max 36px)
        display.drawText(x + 26, textY, tempStr);
        // Col 3: 天气 (Max 48px)
        display.drawText(x + 62, textY, weatherStr);
        // Col 4: 图标 (Fixed 16px)
        display.setDrawColor(1);
        display.drawIcon(x + 112, rowBaseY - 1, 16, 16, iconPtr);
    }
}

void WeatherApp::renderSlotsView() {
    int baseX = (int)slideX + 128;
    int L = AppData.languageIndex;
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
            display.drawText(baseX + 4, drawY, "Empty"); 
        } else {
            if (i == activeSlotIndex) snprintf(buf, sizeof(buf), "* %s", slots[i].name);
            else snprintf(buf, sizeof(buf), "  %s", slots[i].name);
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
            display.drawText(baseX + splitX + 8, 55, "ADD");
        } 
        // 情况 B: 这是一个已有城市
        else {
            // 先检查城市数量
            int count = getCityCount();
            
            if (count <= 1) {
                // 只有这一个独苗 -> 显示锁 (禁止删除) ---
                // 画一个简单的锁头
                display.drawFrame(baseX + rightCenter - 4, midY - 2, 8, 6); // 锁身
                display.drawBox(baseX + rightCenter - 4, midY - 2, 8, 6);   // 实心锁身
                display.drawLine(baseX + rightCenter - 2, midY - 2, baseX + rightCenter - 2, midY - 5); // 锁梁左
                display.drawLine(baseX + rightCenter + 2, midY - 2, baseX + rightCenter + 2, midY - 5); // 锁梁右
                display.drawLine(baseX + rightCenter - 2, midY - 5, baseX + rightCenter + 2, midY - 5); // 锁梁顶
                
                display.drawText(baseX + splitX + 6, 56, "LOCK");
            } 
            else {
                // 可以删除 -> 显示进度条和提示
                // 1. 文字提示 (HOLD / DEL)
                // Y=38 左右显示文字
                if (deleteProgress > 0) {
                    display.drawText(baseX + splitX + 8, 38, "DEL");
                } else {
                    display.drawText(baseX + splitX + 6, 38, "HOLD");
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
        display.drawText(baseX + splitX + 6, 40, "EXIT");
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
    display.drawText(backBtnX, 12, "< Back");
    display.setDrawColor(1); 
    display.drawLine(baseX, 14, baseX+128, 14);
}

void WeatherApp::renderLibraryView() {
    int baseX = (int)slideX + 256;
    int L = AppData.languageIndex;
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
    display.drawText(backBtnX, 12, "< Back");

    display.setDrawColor(1);
    display.drawLine(baseX, 14, baseX+128, 14);
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