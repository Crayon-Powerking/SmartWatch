#include <Arduino.h>
#include "AppConfig.h"
#include "hal/DisplayHAL.h" // 引入我们刚才写的驱动头文件

DisplayHAL display;

void setup() {
    // 初始化串口，方便调试
    Serial.begin(115200);
    Serial.printf("\n[%s] System Booting...\n", PROJECT_NAME);

    // 2. 初始化屏幕硬件
    Serial.println("[HAL] Initializing Display...");
    display.begin();

    // 3. 绘制测试画面
    // 注意：U8g2 是“页缓冲”或“全缓冲”模式，必须调用 update/sendBuffer 才会显示
    display.clear();
    
    // 画两行字
    display.drawText(5, 20, "Hello GitHub!");
    display.drawText(5, 45, "ESP32 Ready");
    
    // 提交到屏幕
    display.update();

    Serial.println("[HAL] Display Content Updated.");
}

void loop() {
    // 暂时留空，只做简单的呼吸灯或日志，证明系统没死机
    // delay(1000);
    // Serial.println("Tick...");
}