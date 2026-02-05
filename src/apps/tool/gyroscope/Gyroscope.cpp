#include "Gyroscope.h"
#include <math.h>

#define SENSITIVITY    1.5f   // 灵敏度：数值越大，小球移动越快
#define MAX_VISIBLE_ANGLE 20  // 超过20度小球就碰壁
#define CENTER_X       64     // 屏幕中心 X
#define CENTER_Y       31     // 屏幕中心 Y
#define BIG_RADIUS     31     // 大圆半径
#define BALL_RADIUS    4      // 小球半径

void Gyroscope::onRun(AppController* sys){
    this->sys = sys;
    this->isGraphMode = true;
    this->isExiting = false;

    sys->btnUp.attachClick([this]{
        this->isGraphMode = !this->isGraphMode;
    });

    sys->btnDown.attachClick([this](){
        this->isGraphMode = !this->isGraphMode;
    });

    sys->btnSelect.attachClick([this](){
        this->isExiting = true;
    });

    sys->btnSelect.attachLongPress(nullptr);
    sys->btnUp.attachDuringLongPress(nullptr);
    sys->btnDown.attachDuringLongPress(nullptr);
}

int Gyroscope::onLoop() {
    if(isGraphMode){
        drawGraphicMode();
    } else {
        drawDataMode();
    }
    if(isExiting)  return 1;
    return 0;
}

void Gyroscope::onExit() {

}

void Gyroscope::drawGraphicMode() {
    ImuData data = sys->imu.getData();

    float offsetX = -data.roll * SENSITIVITY;
    float offsetY = data.pitch * SENSITIVITY;
    
    // 计算当前偏移量距离圆心的长度
    float dist = sqrt(offsetX * offsetX + offsetY * offsetY);
    
    // 允许的最大活动半径 = 大圆半径 - 小球半径 - 边距(如果有)
    float maxDist = BIG_RADIUS - BALL_RADIUS - 1;

    // 如果计算出的距离超过了最大半径，需要按比例缩小向量
    if (dist > maxDist) {
        float scale = maxDist / dist;
        offsetX *= scale;
        offsetY *= scale;
    }

    // 4. 开始绘图
    sys->display.clear(); // 清除缓冲区

    // 绘制外圈空心大圆 (半径32)
    sys->display.drawCircle(CENTER_X, CENTER_Y, BIG_RADIUS);
    
    // 绘制中心的小十字或者中心点，方便用户对准 (可选)
    sys->display.drawPixel(CENTER_X, CENTER_Y);
    sys->display.drawPixel(CENTER_X-1, CENTER_Y);
    sys->display.drawPixel(CENTER_X+1, CENTER_Y);
    sys->display.drawPixel(CENTER_X, CENTER_Y-1);
    sys->display.drawPixel(CENTER_X, CENTER_Y+1);

    // 绘制实心小球
    // 这里的坐标 = 中心点 + 偏移量
    sys->display.drawDisc(CENTER_X + (int)offsetX, CENTER_Y + (int)offsetY, BALL_RADIUS);

    // 提交显示
    sys->display.update();
}

void Gyroscope::drawDataMode() {
    ImuData data = sys->imu.getData();
    sys->display.clear();

    char buf[32];
    sys->display.setFont(u8g2_font_5x7_mf);
    sys->display.setFontMode(1); // 透明背景

    snprintf(buf, sizeof(buf), "Ax: %.2f", data.ax);
    sys->display.drawText(0, 10, buf);
    snprintf(buf, sizeof(buf), "Ay: %.2f", data.ay);
    sys->display.drawText(0, 20, buf);
    snprintf(buf, sizeof(buf), "Az: %.2f", data.az);
    sys->display.drawText(0, 30, buf);

    snprintf(buf, sizeof(buf), "Gx: %.2f", data.gx);
    sys->display.drawText(64, 10, buf);
    snprintf(buf, sizeof(buf), "Gy: %.2f", data.gy);
    sys->display.drawText(64, 20, buf);
    snprintf(buf, sizeof(buf), "Gz: %.2f", data.gz);
    sys->display.drawText(64, 30, buf);

    snprintf(buf, sizeof(buf), "Roll: %.2f", data.roll);
    sys->display.drawText(0, 50, buf);
    snprintf(buf, sizeof(buf), "Pitch: %.2f", data.pitch);
    sys->display.drawText(64, 50, buf);

    sys->display.update();
}