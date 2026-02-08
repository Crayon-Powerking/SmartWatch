#pragma once

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// -- 数据结构 --------------------------------------------------------------------
struct ImuData {
    float ax, ay, az; // 加速度 (m/s^2)
    float gx, gy, gz; // 陀螺仪 (rad/s)
    float temp;       // 温度 (C)
    
    // 解算后的姿态角 (单位：度)
    float roll;       // 翻滚角 (左右倾斜)
    float pitch;      // 俯仰角 (前后倾斜)
};

// -- 类定义 ----------------------------------------------------------------------
class ImuHAL {
public:
    ImuHAL();
    
    bool begin();
    void update();
    ImuData getData() { return data; }
    bool isLiftWrist();
    bool checkStep();

private:
    Adafruit_MPU6050 mpu;
    ImuData data;
    
    const float filterAlpha = 0.8f;         // 互补滤波系数
    const float STEP_THRESHOLD = 11.5f;     // 阈值 (重力是9.8, 超过11.5认为有震动)
    const int   STEP_DELAY_MS = 350;        // 防抖时间 (两步之间至少间隔350ms)

    void detectStep(float ax, float ay, float az); // 内部算法函数
    bool newStepDetected = false;                  // 是否有新步数标志
    unsigned long lastStepTime = 0;                // 上一步的时间戳
};