#pragma once
#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// IMU 数据结构
struct ImuData {
    float ax, ay, az; // 加速度 (m/s^2)
    float gx, gy, gz; // 陀螺仪 (rad/s)
    float temp;       // 温度 (C)
    // 解算后的姿态角 (单位：度)
    float roll;       // 翻滚角 (左右倾斜)
    float pitch;      // 俯仰角 (前后倾斜)
};

class ImuHAL {
public:
    ImuHAL();
    bool begin();     
    void update();    
    ImuData getData() { return data; }
    bool isLiftWrist();

private:
    Adafruit_MPU6050 mpu;
    ImuData data;
    const float filterAlpha = 0.8f;  // 互补滤波系数
};