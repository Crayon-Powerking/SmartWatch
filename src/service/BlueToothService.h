#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <functional>

// -- 基础定义 --------------------------------------------------------------------
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" //
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

struct BLEConfigData {
    bool hasWifi = false;
    bool hasKey  = false;

    char ssid[32] = ""; 
    char pass[64] = "";
    char key[64] = "";
};

// -- 类定义 ----------------------------------------------------------------------
class BluetoothService {
public:
    BluetoothService();

    // 定义回调类型：一个接收 BLEConfigData，返回 void 的函数
    typedef std::function<void(BLEConfigData)> ConfigCallback;

    void begin(const char* deviceName); // 开启蓝牙
    void stop();                        // 关闭蓝牙

    bool isServiceRunning() { return _isServiceRunning; } // 蓝牙开关状态
    // 注册回调函数
    void setConfigCallback(ConfigCallback cb);

    bool isConnected();                 // 手机是否已连接

private:
    BLEServer* pServer = nullptr;
    
    // 回调函数存储变量
    ConfigCallback _dataCallback = nullptr;
    
    // 状态管理
    bool _deviceConnected = false;  // 蓝牙连接状态
    bool _isServiceRunning = false; // 蓝牙开启状态
    bool _rxReady = false;          // 收到原始数据标记
    String _rxBuffer = "";          // 原始 JSON 字符串
    
    // 后台任务
    TaskHandle_t _parseTaskHandle = nullptr;
    static void parseTask(void* parameter); 
    void doParse();                 // 执行解析逻辑

    friend class MyServerCallbacks;
    friend class MyCallbacks;
};