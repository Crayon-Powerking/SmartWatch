#include "AlarmApp.h"
#include "assets/Lang.h"
#include <cmath>

// -- 辅助函数 (数据、排序、逻辑助手) -------------------------------------------

void AlarmApp::saveAlarmData() {
    if (sys) {
        sys->storage.saveStruct("alarm_data", AppData.alarmSlots);
    }
}

void AlarmApp::loadAlarmData() {
    if (sys) {
        sys->storage.loadStruct("alarm_data", AppData.alarmSlots);
    }
}

int AlarmApp::getAlarmCount() {
    int count = 0;
    for (int i = 0; i < MAX_ALARMS; i++) {
        if (AppData.alarmSlots[i].isUsed) count++;
    }
    return count;
}

void AlarmApp::initNewAlarm(int index) {
    if (index >= 0 && index < MAX_ALARMS) {
        AppData.alarmSlots[index].isUsed = true;
        AppData.alarmSlots[index].isOpen = true;
        AppData.alarmSlots[index].hour = 7;
        AppData.alarmSlots[index].minute = 0;
        AppData.alarmSlots[index].weekMask = DEFAULT_WEEK_MASK;
    }
}

void AlarmApp::sortAlarms() {
    for (int i = 0; i < MAX_ALARMS - 1; i++) {
        for (int j = 0; j < MAX_ALARMS - 1 - i; j++) {
            bool swap = false;
            if (!AppData.alarmSlots[j].isUsed && AppData.alarmSlots[j + 1].isUsed) {
                swap = true;
            } else if (AppData.alarmSlots[j].isUsed && AppData.alarmSlots[j + 1].isUsed) {
                int timeA = AppData.alarmSlots[j].hour * 60 + AppData.alarmSlots[j].minute;
                int timeB = AppData.alarmSlots[j + 1].hour * 60 + AppData.alarmSlots[j + 1].minute;
                if (timeA > timeB) swap = true;
            }
            if (swap) {
                AlarmSlot temp = AppData.alarmSlots[j];
                AppData.alarmSlots[j] = AppData.alarmSlots[j + 1];
                AppData.alarmSlots[j + 1] = temp;
            }
        }
    }
}

void AlarmApp::handleLongPressLoop(float dt) {
    bool isPressed = sys->btnSelect.isPressed();
    if (!isPressed) {
        deleteProgress = 0.0f;
        pressStartTime = 0;
        return;
    }

    if (pressStartTime == 0) {
        pressStartTime = millis();
    }
    unsigned long holdTime = millis() - pressStartTime;

    if (holdTime < 300 || suppressClick) {
        return;
    }

    int count = getAlarmCount();
    if (viewState == VIEW_LIST && listIndex >= 0 && listIndex < count) {
        deleteProgress += DELETE_SPEED * dt;
        if (deleteProgress >= 1.0f) {
            AppData.alarmSlots[listIndex].isOpen = false;
            AppData.alarmSlots[listIndex].isUsed = false;
            sortAlarms();

            int newCount = getAlarmCount();
            if (listIndex >= newCount) listIndex = (newCount > 0) ? newCount - 1 : 0;

            deleteProgress = 0.0f;
            suppressClick = true;
            sys->processInput();
        }
    } else if (viewState == VIEW_EDIT && editFocus == FOCUS_WEEK_ROW && isEditingWeekRow) {
        deleteProgress += DELETE_SPEED * dt;
        if (deleteProgress >= 1.0f) {
            isEditingWeekRow = false;
            deleteProgress = 0.0f;
            suppressClick = true;
            if (!sys->processInput()) return;
        }
    }
}

// -- 按键输入函数 -------------------------------------------------------------

void AlarmApp::onkeyUp() {
    if (viewState == VIEW_LIST) {
        if (listIndex > -1) listIndex--;
    } else if (viewState == VIEW_EDIT) {
        if (isEditingValue) {
            if (editFocus == FOCUS_TIME_HOUR) {
                AppData.alarmSlots[listIndex].hour++;
                if (AppData.alarmSlots[listIndex].hour >= 24) AppData.alarmSlots[listIndex].hour = 0;
            } else if (editFocus == FOCUS_TIME_MINUTE) {
                AppData.alarmSlots[listIndex].minute++;
                if (AppData.alarmSlots[listIndex].minute >= 60) AppData.alarmSlots[listIndex].minute = 0;
            }
        } else if (editFocus == FOCUS_WEEK_ROW && isEditingWeekRow) {
            weekDayCursor++;
            if (weekDayCursor > 6) weekDayCursor = 0;
        } else {
            int focus = (int)editFocus;
            focus--;
            if (focus < 0) focus = FOCUS_WEEK_ROW;
            editFocus = (EditFocus)focus;
        }
    }
}

void AlarmApp::onkeyDown() {
    int count = getAlarmCount();
    if (viewState == VIEW_LIST) {
        int maxIdx = (count < MAX_ALARMS) ? count : count - 1;
        if (listIndex < maxIdx) listIndex++;
    } else if (viewState == VIEW_EDIT) {
        if (isEditingValue) {
            if (editFocus == FOCUS_TIME_HOUR) {
                if (AppData.alarmSlots[listIndex].hour == 0) AppData.alarmSlots[listIndex].hour = 23;
                else AppData.alarmSlots[listIndex].hour--;
            } else if (editFocus == FOCUS_TIME_MINUTE) {
                if (AppData.alarmSlots[listIndex].minute == 0) AppData.alarmSlots[listIndex].minute = 59;
                else AppData.alarmSlots[listIndex].minute--;
            }
        } else if (editFocus == FOCUS_WEEK_ROW && isEditingWeekRow) {
            weekDayCursor--;
            if (weekDayCursor < 0) weekDayCursor = 6;
        } else {
            int focus = (int)editFocus;
            focus++;
            if (focus >= FOCUS_MAX) focus = FOCUS_BACK_BTN;
            editFocus = (EditFocus)focus;
        }
    }
}

void AlarmApp::onkeySelect() {
    if (viewState == VIEW_LIST) {
        if (listIndex == -1) {
            this->isExiting = true;
        } else {
            int count = getAlarmCount();
            if (count < MAX_ALARMS && listIndex == count) {
                initNewAlarm(count);
            }
            viewState = VIEW_EDIT;
            editFocus = FOCUS_TIME_HOUR;
            isEditingValue = false;
            isEditingWeekRow = false;
        }
    } else if (viewState == VIEW_EDIT) {
        if (editFocus == FOCUS_BACK_BTN) {
            sortAlarms();
            viewState = VIEW_LIST;
        } else if (editFocus == FOCUS_SWITCH) {
            AppData.alarmSlots[listIndex].isOpen = !AppData.alarmSlots[listIndex].isOpen;
        } else if (editFocus == FOCUS_TIME_HOUR || editFocus == FOCUS_TIME_MINUTE) {
            isEditingValue = !isEditingValue;
        } else if (editFocus == FOCUS_WEEK_ROW) {
            if (isEditingWeekRow) {
                AppData.alarmSlots[listIndex].weekMask ^= (1 << weekDayCursor);
            } else {
                isEditingWeekRow = true;
            }
        }
    }
}

// -- 生命周期 (Run, Loop, Exit) -----------------------------------------------

void AlarmApp::onRun(AppController* sys) {
    this->sys = sys;

    loadAlarmData();
    sortAlarms();

    viewState = VIEW_LIST;
    listIndex = -1;
    selectionSmooth = -1;
    scrollY = 0;
    weekDayCursor = 1;
    deleteProgress = 0.0f;
    slideX = 0;
    pressStartTime = 0;
    suppressClick = false;

    sys->btnUp.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onkeyUp();
    });
    sys->btnDown.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onkeyDown();
    });

    sys->btnSelect.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        if (suppressClick) {
            suppressClick = false;
            return;
        }
        this->onkeySelect();
    });

    sys->btnUp.attachDuringLongPress([this, sys] {
        static unsigned long lastTrig = 0;
        if (!sys->processInput()) return;
        if (millis() - lastTrig > 100) {
            lastTrig = millis();
            this->onkeyUp();
        }
    });

    sys->btnDown.attachDuringLongPress([this, sys] {
        static unsigned long lastTrig = 0;
        if (!sys->processInput()) return;
        if (millis() - lastTrig > 100) {
            lastTrig = millis();
            this->onkeyDown();
        }
    });
}

int AlarmApp::onLoop() {
    if (this->isExiting) return 1;

    unsigned long now = millis();
    float dt = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;

    handleLongPressLoop(dt);

    float diffIdx = (float)listIndex - selectionSmooth;
    if (fabs(diffIdx) > 0.01f) {
        selectionSmooth += diffIdx * 0.25f;
    } else {
        selectionSmooth = (float)listIndex;
    }

    if (viewState == VIEW_LIST) {
        float cursorTargetY = (listIndex < 0) ? 0 : selectionSmooth * LIST_ITEM_H;
        float targetCamY = cursorTargetY - 24 + 8;

        if (targetCamY < 0) targetCamY = 0;
        int count = getAlarmCount();
        int totalRows = (count < MAX_ALARMS) ? count + 1 : count;
        float maxScroll = (totalRows * LIST_ITEM_H) - 48 + 10;
        if (maxScroll < 0) maxScroll = 0;
        if (targetCamY > maxScroll) targetCamY = maxScroll;

        scrollY += (targetCamY - scrollY) * 0.2f;
    }

    int targetX = (viewState == VIEW_LIST) ? 0 : -128;
    float diffX = targetX - slideX;
    if (fabs(diffX) > 0.5f) slideX += diffX * 0.3f;
    else slideX = targetX;

    render();
    return 0;
}

void AlarmApp::onExit() {
    saveAlarmData();
}

// -- 绘图渲染函数 -------------------------------------------------------------

void AlarmApp::render() {
    sys->display.clear();
    if (slideX > -128) drawListView((int)slideX);
    if (slideX < 0) drawEditView((int)slideX + 128);
}

void AlarmApp::drawListView(int offsetX) {
    int L = AppData.systemConfig.languageIndex;
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);
    sys->display.setFontMode(1);
    sys->display.setDrawColor(1);

    sys->display.drawLine(offsetX, HEADER_H - 1, offsetX + 128, HEADER_H - 1);
    sys->display.drawLine(offsetX + SPLIT_X, 0, offsetX + SPLIT_X, 64);
    sys->display.drawText(offsetX + 4, 11, STR_ALARM_TITLE[L]);

    if (listIndex == -1) {
        sys->display.drawBox(offsetX + SPLIT_X + 1, 0, 128 - SPLIT_X, HEADER_H);
        sys->display.setDrawColor(0);
    }

    int strW = sys->display.getStrWidth(STR_BACK[L]);
    int boxCenter = SPLIT_X + (128 - SPLIT_X) / 2;
    sys->display.drawText(offsetX + boxCenter - (strW / 2), 11, STR_BACK[L]);
    sys->display.setDrawColor(1);

    sys->display.setClipWindow(offsetX, HEADER_H, offsetX + SPLIT_X, 64);

    int count = getAlarmCount();
    int totalItems = (count < MAX_ALARMS) ? count + 1 : count;

    if (selectionSmooth > -0.8f) {
        int cursorY = HEADER_H + (int)(selectionSmooth * LIST_ITEM_H) - (int)scrollY;
        if (cursorY > HEADER_H - LIST_ITEM_H && cursorY < 64) {
            sys->display.setDrawColor(1);
            sys->display.drawRBox(offsetX + 2, cursorY + 1, SPLIT_X - 4, LIST_ITEM_H - 2, 2);
        }
    }

    for (int i = 0; i < totalItems; i++) {
        int drawY = HEADER_H + (i * LIST_ITEM_H) - (int)scrollY;
        if (drawY > 64 || drawY < HEADER_H - LIST_ITEM_H) continue;
        bool isSelected = (listIndex != -1 && (int)(selectionSmooth + 0.5f) == i);

        sys->display.setDrawColor(isSelected ? 0 : 1);

        if (i < count) {
            char buf[16];
            sprintf(buf, "%02d:%02d", AppData.alarmSlots[i].hour, AppData.alarmSlots[i].minute);
            sys->display.drawText(offsetX + 6, drawY + 11, buf);
        } else {
            sys->display.drawText(offsetX + 6, drawY + 11, STR_ALARM_ADD[L]);
        }
    }
    sys->display.setMaxClipWindow();
    sys->display.setDrawColor(1);

    if (listIndex >= -1 && listIndex < totalItems) {
        int centerX = offsetX + RIGHT_CENTER;
        int centerY = 40;

        if (listIndex == -1) {
            sys->display.drawText(centerX - 9, centerY + 16, STR_EXIT[L]);
            return;
        }

        if (listIndex == count) {
            sys->display.drawLine(centerX - 4, centerY, centerX + 4, centerY);
            sys->display.drawLine(centerX, centerY - 4, centerX, centerY + 4);
            sys->display.drawText(centerX - 9, centerY + 16, STR_ALARM_NEW[L]);
        } else {
            if (deleteProgress > 0.1f) {
                sys->display.drawText(centerX - 9, 28, STR_DEL[L]);
                int barW = 6, barH = 20, barX = centerX - 3, barY = 32;
                sys->display.drawFrame(barX, barY, barW, barH);
                int fillH = (int)(deleteProgress * barH);
                if (fillH > barH) fillH = barH;
                sys->display.drawBox(barX, barY + barH - fillH, barW, fillH);
            } else {
                bool isOpen = AppData.alarmSlots[listIndex].isOpen;
                sys->display.drawText(centerX - 9, centerY + 16, isOpen ? STR_ALARM_ON[L] : STR_ALARM_OFF[L]);
            }
        }
    }
}

void AlarmApp::drawEditView(int offsetX) {
    int L = AppData.systemConfig.languageIndex;
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);
    sys->display.setDrawColor(1);
    sys->display.drawLine(offsetX, 15, offsetX + 128, 15);

    if (editFocus == FOCUS_BACK_BTN) {
        sys->display.drawBox(offsetX, 0, 39, 14);
        sys->display.setDrawColor(0);
    }
    sys->display.drawText(offsetX + 2, 11, STR_BACK[L]);
    sys->display.setDrawColor(1);

    int boxX = offsetX + 88;
    const char* swStr = AppData.alarmSlots[listIndex].isOpen ? STR_ALARM_ON[L] : STR_ALARM_OFF[L];
    if (editFocus == FOCUS_SWITCH) {
        sys->display.drawRFrame(boxX, 0, 38, 14, 2);
    }
    sys->display.drawText(boxX + (38 - sys->display.getStrWidth(swStr)) / 2, 11, swStr);

    sys->display.setFont(u8g2_font_logisoso24_tn);
    char hourBuf[4], minBuf[4];
    sprintf(hourBuf, "%02d", AppData.alarmSlots[listIndex].hour);
    sprintf(minBuf, "%02d", AppData.alarmSlots[listIndex].minute);

    int hW = sys->display.getStrWidth(hourBuf);
    int mW = sys->display.getStrWidth(minBuf);
    int colonW = sys->display.getStrWidth(":");
    int gap = 4;
    int totalTimeW = hW + colonW + mW + gap * 2;
    int startTimeX = offsetX + (128 - totalTimeW) / 2;
    int timeBaseY = 42, focusBoxY = 17, focusBoxH = 27, lineY = 45;

    // 小时
    if (editFocus == FOCUS_TIME_HOUR) {
        if (isEditingValue) {
            sys->display.drawBox(startTimeX - 2, focusBoxY, hW + 4, focusBoxH);
            sys->display.setDrawColor(0);
        } else {
            sys->display.drawLine(startTimeX - 1, lineY, startTimeX + hW + 1, lineY);
        }
    }
    sys->display.drawText(startTimeX, timeBaseY, hourBuf);
    sys->display.setDrawColor(1);
    sys->display.drawText(startTimeX + hW + gap, timeBaseY - 2, ":");

    // 分钟
    int minX = startTimeX + hW + colonW + gap * 2;
    if (editFocus == FOCUS_TIME_MINUTE) {
        if (isEditingValue) {
            sys->display.drawBox(minX - 2, focusBoxY, mW + 4, focusBoxH);
            sys->display.setDrawColor(0);
        } else {
            sys->display.drawLine(minX - 1, lineY, minX + mW + 1, lineY);
        }
    }
    sys->display.drawText(minX, timeBaseY, minBuf);
    sys->display.setDrawColor(1);

    // 星期
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);
    const char* weeks[] = {"S", "M", "T", "W", "T", "F", "S"};
    int wStartX = 10, wGap = 16, weekY = 60;

    if (editFocus == FOCUS_WEEK_ROW && !isEditingWeekRow) {
        sys->display.drawFrame(offsetX + 6, 48, 116, 15);
    }

    for (int i = 0; i < 7; i++) {
        int x = offsetX + wStartX + i * wGap;
        bool isActive = (AppData.alarmSlots[listIndex].weekMask >> i) & 1;

        if (isActive) {
            sys->display.setDrawColor(1);
            sys->display.drawBox(x - 1, 49, 13, 12);
            sys->display.setDrawColor(0);
        }

        int charW = sys->display.getStrWidth(weeks[i]);
        sys->display.drawText(x + (11 - charW) / 2, weekY, weeks[i]);
        sys->display.setDrawColor(1);

        if (isEditingWeekRow && weekDayCursor == i) {
            sys->display.drawFrame(x - 2, 48, 15, 14);
        }
    }

    if (isEditingWeekRow && deleteProgress > 0.05f) {
        int barW = (int)(116 * deleteProgress);
        sys->display.drawBox(offsetX + 6, 62, barW, 2);
    }
}