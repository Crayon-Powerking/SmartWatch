#include "apps/tool/gyroscope/Gyroscope.h"
#include <cmath>
#include <cstdio>

// -- 按键处理 --------------------------------------------------------------------

void Gyroscope::onKeyUp() {
    this->isGraphMode = !this->isGraphMode;
}

void Gyroscope::onKeyDown() {
    this->isGraphMode = !this->isGraphMode;
}

void Gyroscope::onKeySelect() {
    this->isExiting = true;
}

// -- 生命周期 --------------------------------------------------------------------

void Gyroscope::onRun(AppController* sys) {
    this->sys = sys;
    this->isGraphMode = true;
    this->isExiting = false;

    // 绑定按键回调
    sys->btnUp.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeyUp();
    });

    sys->btnDown.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeyDown();
    });

    sys->btnSelect.attachClick([this, sys]() {
        if (!sys->processInput()) return;
        this->onKeySelect();
    });

    // 禁用长按功能
    sys->btnSelect.attachLongPress(nullptr);
    sys->btnUp.attachDuringLongPress(nullptr);
    sys->btnDown.attachDuringLongPress(nullptr);
}

int Gyroscope::onLoop() {
    if (this->isExiting) return 1;

    render();
    return 0;
}

void Gyroscope::onExit() {
    sys->display.setFont(u8g2_font_wqy12_t_gb2312);
}

// -- 绘图渲染 --------------------------------------------------------------------

void Gyroscope::render() {
    sys->display.clear();

    if (isGraphMode) {
        drawGraphicMode();
    } else {
        drawDataMode();
    }
}

void Gyroscope::drawGraphicMode() {
    ImuData data = sys->imu.getData();

    // 1. 计算偏移量
    float offsetX = -data.roll * SENSITIVITY;
    float offsetY = data.pitch * SENSITIVITY;

    // 2. 限制在圆内
    float dist = sqrt(offsetX * offsetX + offsetY * offsetY);
    float maxDist = BIG_RADIUS - BALL_RADIUS - 1;

    if (dist > maxDist) {
        float scale = maxDist / dist;
        offsetX *= scale;
        offsetY *= scale;
    }

    // 3. 绘制外圈空心大圆
    sys->display.setDrawColor(1);
    sys->display.drawCircle(CENTER_X, CENTER_Y, BIG_RADIUS);

    // 4. 绘制中心准星
    sys->display.drawPixel(CENTER_X, CENTER_Y);
    sys->display.drawPixel(CENTER_X - 1, CENTER_Y);
    sys->display.drawPixel(CENTER_X + 1, CENTER_Y);
    sys->display.drawPixel(CENTER_X, CENTER_Y - 1);
    sys->display.drawPixel(CENTER_X, CENTER_Y + 1);

    // 5. 绘制实心小球 (中心 + 偏移)
    sys->display.drawDisc(CENTER_X + (int)offsetX, CENTER_Y + (int)offsetY, BALL_RADIUS);
}

void Gyroscope::drawDataMode() {
    ImuData data = sys->imu.getData();
    char buf[32];

    sys->display.setFont(u8g2_font_5x7_mf);
    sys->display.setFontMode(1);
    sys->display.setDrawColor(1);

    // 加速度数据
    snprintf(buf, sizeof(buf), "Ax: %.2f", data.ax);
    sys->display.drawText(0, 10, buf);
    snprintf(buf, sizeof(buf), "Ay: %.2f", data.ay);
    sys->display.drawText(0, 20, buf);
    snprintf(buf, sizeof(buf), "Az: %.2f", data.az);
    sys->display.drawText(0, 30, buf);

    // 角速度数据
    snprintf(buf, sizeof(buf), "Gx: %.2f", data.gx);
    sys->display.drawText(64, 10, buf);
    snprintf(buf, sizeof(buf), "Gy: %.2f", data.gy);
    sys->display.drawText(64, 20, buf);
    snprintf(buf, sizeof(buf), "Gz: %.2f", data.gz);
    sys->display.drawText(64, 30, buf);

    // 姿态解算数据
    snprintf(buf, sizeof(buf), "Roll: %.2f", data.roll);
    sys->display.drawText(0, 50, buf);
    snprintf(buf, sizeof(buf), "Pitch: %.2f", data.pitch);
    sys->display.drawText(64, 50, buf);
}