#pragma once

// 定义数据模型结构体
struct AppDataModel {
    // 1. 运动数据
    int stepCount = 0;
    
    // 2. 模拟电量 (0-100)
    int batteryLevel = 80; 

    // 3. UI 状态
    // 0 = 步数页, 1 = 电量页
    int currentScreen = 0; 
    
    // 构造函数: 给个初值
    AppDataModel() {
        stepCount = 0;
        batteryLevel = 80;
        currentScreen = 0;
    }
};

// 声明一个全局变量，让所有文件都能找到它
extern AppDataModel AppData;