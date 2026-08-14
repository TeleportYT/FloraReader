#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BLEManager {
public:
    BLEManager();
    void begin();
    void stop();
    bool isConnected() const { return m_deviceConnected; }

private:
    bool m_deviceConnected;
    bool m_initialized;
    BLEServer* m_pServer;
    BLECharacteristic* m_pTxCharacteristic;
};

#endif // BLE_MANAGER_H
