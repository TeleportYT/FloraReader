#include "BLEManager.h"

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // Nordic UART Service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class ServerCallbacks: public BLEServerCallbacks {
public:
    ServerCallbacks(bool& connected) : m_connected(connected) {}
    void onConnect(BLEServer* pServer) override { m_connected = true; }
    void onDisconnect(BLEServer* pServer) override { m_connected = false; }
private:
    bool& m_connected;
};

BLEManager::BLEManager() : m_deviceConnected(false), m_initialized(false), m_pServer(nullptr), m_pTxCharacteristic(nullptr) {
}

void BLEManager::begin() {
    if (!m_initialized) {
        // First-time full initialization
        BLEDevice::init("FloraReader-BLE");
        m_pServer = BLEDevice::createServer();
        m_pServer->setCallbacks(new ServerCallbacks(m_deviceConnected));

        BLEService *pService = m_pServer->createService(SERVICE_UUID);
        m_pTxCharacteristic = pService->createCharacteristic(
                                CHARACTERISTIC_UUID_TX,
                                BLECharacteristic::PROPERTY_NOTIFY
                              );
        m_pTxCharacteristic->addDescriptor(new BLE2902());

        pService->start();
        m_initialized = true;
        Serial.println("[BLE] Bluetooth BLE service initialized: FloraReader-BLE");
    }

    // Start (or restart) advertising
    m_pServer->getAdvertising()->start();
    Serial.println("[BLE] Bluetooth advertising started.");
}

void BLEManager::stop() {
    if (m_initialized && m_pServer) {
        m_pServer->getAdvertising()->stop();
        Serial.println("[BLE] Bluetooth advertising stopped.");
    }
    m_deviceConnected = false;
}
