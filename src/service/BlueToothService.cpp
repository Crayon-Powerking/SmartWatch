#include "service/BluetoothService.h"

// -- BLE 回调处理 ---------------------------------------------------------------

class MyServerCallbacks : public BLEServerCallbacks {
    BluetoothService* _service;
public:
    MyServerCallbacks(BluetoothService* service) : _service(service) {}
    void onConnect(BLEServer* pServer) { _service->_deviceConnected = true; }
    void onDisconnect(BLEServer* pServer) { 
        _service->_deviceConnected = false; 
        BLEDevice::startAdvertising(); // 断开后自动恢复广播
    }
};

class MyCallbacks : public BLECharacteristicCallbacks {
    BluetoothService* _service;
public:
    MyCallbacks(BluetoothService* service) : _service(service) {}
    void onWrite(BLECharacteristic* pCharacteristic) {
        String val = pCharacteristic->getValue().c_str(); 
        if (val.length() > 0) {
            _service->_rxBuffer = val; // 存入缓冲
            _service->_rxReady = true; // 通知后台任务干活
        }
    }
};

// -- 服务实现 --------------------------------------------------------------------

BluetoothService::BluetoothService() {}

void BluetoothService::begin(const char* deviceName) {
    if (_isServiceRunning) return;
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks(this));

    BLEService* pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic* pRx = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE
    );
    pRx->setCallbacks(new MyCallbacks(this));

    pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);

    pService->start();
    BLEDevice::startAdvertising();

    if (_parseTaskHandle == NULL) {
        xTaskCreate(parseTask, "BT_Parse", 4096, this, 1, &_parseTaskHandle);
    }
    _isServiceRunning = true;
}

void BluetoothService::stop() {
    if (!_isServiceRunning) return;
    if (_parseTaskHandle != NULL) {
        vTaskDelete(_parseTaskHandle);
        _parseTaskHandle = NULL;
    }
    BLEDevice::deinit();
    _isServiceRunning = false; 
    _deviceConnected = false;
}

void BluetoothService::setConfigCallback(ConfigCallback cb) {
    _dataCallback = cb;
}

bool BluetoothService::isConnected() { return _deviceConnected; }

// -- 后台解析逻辑 ----------------------------------------------------------------

void BluetoothService::parseTask(void* parameter) {
    BluetoothService* self = (BluetoothService*)parameter;
    while (true) {
        if (self->_rxReady) {
            self->doParse();
            self->_rxReady = false;
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); 
    }
}

void BluetoothService::doParse() {
    JsonDocument doc;
    // 尝试解析 JSON
    DeserializationError error = deserializeJson(doc, _rxBuffer);

    if (error == DeserializationError::Ok) {
        BLEConfigData parsedData; 
        
        // 1. 检查 WiFi
        if (doc["ssid"].is<const char*>()) {
            const char* s = doc["ssid"];
            const char* p = doc["pass"];
            
            strncpy(parsedData.ssid, s, 31);
            parsedData.ssid[31] = '\0';
            
            if (p) {
                strncpy(parsedData.pass, p, 63);
                parsedData.pass[63] = '\0';
            } else {
                parsedData.pass[0] = '\0';
            }
            parsedData.hasWifi = true;
        }

        // 2. 检查 Weather Key
        if (doc["key"].is<const char*>()) {
            const char* k = doc["key"];
            strncpy(parsedData.key, k, 63);
            parsedData.key[63] = '\0';
            parsedData.hasKey = true;
        }

        if (_dataCallback != nullptr) {
            _dataCallback(parsedData);
        }

    }
    _rxBuffer = ""; // 清空
}