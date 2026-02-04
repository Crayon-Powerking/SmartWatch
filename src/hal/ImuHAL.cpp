#include "hal/ImuHAL.h"
#include "model/AppData.h"
#include <math.h>

ImuHAL::ImuHAL() {
    data = {0};
}

bool ImuHAL::begin() {
    // 初始化 I2C 总线
    Wire.begin(21, 22);

    // 初始化IMU传感器
    if (!mpu.begin()) {
        return false;
    }

    // 设置加速度量程: +-8G
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G); 

    // 设置陀螺仪量程: +-500度/秒
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);      

    // 设置硬件低通滤波器带宽: 21Hz
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);   

    return true;
}

void ImuHAL::update() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    detectStep(a.acceleration.x, a.acceleration.y, a.acceleration.z);
    // 互补滤波处理加速度数据
    data.ax = data.ax * filterAlpha + a.acceleration.x * (1 - filterAlpha);
    data.ay = data.ay * filterAlpha + a.acceleration.y * (1 - filterAlpha);
    data.az = data.az * filterAlpha + a.acceleration.z * (1 - filterAlpha);
    
    // 陀螺仪和温度直接取值
    data.gx = g.gyro.x;
    data.gy = g.gyro.y;
    data.gz = g.gyro.z;
    data.temp = temp.temperature;

    // 简易姿态解算
    // 57.29578 是 180/PI 的值，用于把弧度转角度
    // Pitch (俯仰): 绕 Y 轴旋转
    data.pitch = atan2(-data.ax, sqrt(data.ay * data.ay + data.az * data.az)) * 57.29578;
    
    // Roll (翻滚): 绕 X 轴旋转
    data.roll = atan2(data.ay, data.az) * 57.29578;
}

void ImuHAL::detectStep(float ax, float ay, float az) {
    // 1. 计算合加速度向量长度 (Magnitude)
    // 公式: m = sqrt(x^2 + y^2 + z^2)
    float magnitude = sqrt(ax * ax + ay * ay + az * az);

    // 2. 简单的峰值检测
    // 正常静止时 magnitude 约为 9.8 (1G 重力)
    // 走路时会有冲击，瞬间值会超过 11.0 ~ 13.0
    if (magnitude > STEP_THRESHOLD) {
        unsigned long now = millis();
        // 3. 时间防抖: 人类走路最快约为 2-3 步/秒，所以间隔需 > 300ms
        if (now - lastStepTime > STEP_DELAY_MS) {
            newStepDetected = true;
            lastStepTime = now;
        }
    }
}

bool ImuHAL::checkStep() {
    if (newStepDetected) {
        newStepDetected = false; 
        return true;
    }
    return false;
}

// 抬手亮屏
bool ImuHAL::isLiftWrist() {
    bool isLooking = (data.az > 4.0 && data.ay > 1.5);
    return isLooking;
}