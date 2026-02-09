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
            _service->_rxBuffer = val;
            _service->_rxReady = true; // 通知后台任务处理
        }
    }
};

// -- 服务实现 --------------------------------------------------------------------

BluetoothService::BluetoothService() {}

void BluetoothService::begin(const char* deviceName) {
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks(this));

    BLEService* pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic* pRx = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE
    );
    pRx->setCallbacks(new MyCallbacks(this));

    pService->start();
    BLEDevice::startAdvertising();

    // 开启后台解析任务 (优先级设为1，不干扰显示渲染)
    xTaskCreate(parseTask, "BT_Parse", 4096, this, 1, &_parseTaskHandle);
}

void BluetoothService::stop() {
    if (_parseTaskHandle != NULL) {
        vTaskDelete(_parseTaskHandle);
        _parseTaskHandle = NULL;
    }
    BLEDevice::deinit();
}

// -- 数据接口 --------------------------------------------------------------------

bool BluetoothService::isConnected() { return _deviceConnected; }
bool BluetoothService::isDataReady() { return _parsedData.valid; }

BLEConfigData BluetoothService::getConfig() {
    BLEConfigData data = _parsedData;
    _parsedData.valid = false; // 取走后标记为无效
    return data;
}

void BluetoothService::clearData() {
    _parsedData.valid = false;
    _rxReady = false;
}

// -- 后台解析逻辑 (FreeRTOS 任务) ------------------------------------------------

void BluetoothService::parseTask(void* parameter) {
    BluetoothService* self = (BluetoothService*)parameter;
    while (true) {
        if (self->_rxReady) {
            self->doParse();
            self->_rxReady = false;
        }
        vTaskDelay(200 / portTICK_PERIOD_MS); // 每200ms检查一次
    }
}

void BluetoothService::doParse() {
    JsonDocument doc;
    if (deserializeJson(doc, _rxBuffer) == DeserializationError::Ok) {
        if (doc.containsKey("ssid")) strncpy(_parsedData.ssid, doc["ssid"], 31);
        if (doc.containsKey("pass")) strncpy(_parsedData.pass, doc["pass"], 63);
        if (doc.containsKey("key"))  strncpy(_parsedData.key,  doc["key"],  63);
        if (doc.containsKey("ts"))   _parsedData.timestamp = doc["ts"];
        _parsedData.valid = true;
    }
}