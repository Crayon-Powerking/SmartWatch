#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

// -- 基础定义 --------------------------------------------------------------------
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// 解析后的配置结构体
struct BLEConfigData {
    bool valid = false;
    char ssid[32] = ""; 
    char pass[64] = "";
    char key[64] = "";
    long timestamp = 0;
};

// -- 类定义 ----------------------------------------------------------------------
class BluetoothService {
public:
    BluetoothService();

    void begin(const char* deviceName); 
    void stop();                        

    bool isConnected();                 // 是否连接
    bool isDataReady();                 // 是否收到完整配置
    BLEConfigData getConfig();          // 获取并清除有效标记
    void clearData();                   // 重置状态

private:
    BLEServer* pServer = nullptr;
    
    // 状态管理
    bool _deviceConnected = false;
    bool _rxReady = false;
    String _rxBuffer = "";
    BLEConfigData _parsedData;

    // 后台任务相关 (FreeRTOS)
    TaskHandle_t _parseTaskHandle = nullptr;
    static void parseTask(void* parameter); 
    void doParse(); // 具体的解析逻辑

    friend class MyServerCallbacks;
    friend class MyCallbacks;
};