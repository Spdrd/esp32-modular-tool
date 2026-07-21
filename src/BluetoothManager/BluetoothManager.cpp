#include "BluetoothManager.h"
#include <BluetoothSerial.h>

static BluetoothSerial SerialBT;

BluetoothManager::BluetoothManager(const char* name) : active(false) {
    strncpy(deviceName, name, sizeof(deviceName) - 1);
    deviceName[sizeof(deviceName) - 1] = '\0';
}

bool BluetoothManager::begin() {
    if (active) return true;

    if (!SerialBT.begin(deviceName)) {
        Serial.println("[BT] init failed");
        return false;
    }

    active = true;
    Serial.printf("[BT] ready as '%s'\n", deviceName);
    return true;
}

void BluetoothManager::end() {
    if (!active) return;
    active = false;
    SerialBT.end();
    Serial.println("[BT] ended");
}

bool BluetoothManager::isConnected() const {
    return active && SerialBT.hasClient();
}

void BluetoothManager::send(const char* text) {
    if (!active || !SerialBT.hasClient()) return;
    SerialBT.print(text);
}

int BluetoothManager::available() {
    if (!active || !SerialBT.hasClient()) return 0;
    return SerialBT.available();
}

int BluetoothManager::read() {
    if (!active || !SerialBT.hasClient()) return -1;
    return SerialBT.read();
}

const char* BluetoothManager::getStatus() const {
    if (!active) {
        snprintf(statusBuf, sizeof(statusBuf), "Apagado");
    } else if (!SerialBT.hasClient()) {
        snprintf(statusBuf, sizeof(statusBuf), "Esperando...");
    } else {
        snprintf(statusBuf, sizeof(statusBuf), "Conectado");
    }
    return statusBuf;
}
