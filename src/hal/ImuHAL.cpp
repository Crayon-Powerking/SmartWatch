#include "hal/ImuHAL.h"
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

// 抬手亮屏算法
bool ImuHAL::isLiftWrist() {
    bool isLooking = (data.az > 4.0 && data.ay > 1.5);
    return isLooking;
}