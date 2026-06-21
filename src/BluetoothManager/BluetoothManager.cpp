#include "BluetoothManager.h"
#include <BleKeyboard.h>

static BleKeyboard s_ble("ESP32-Remote", "ESP32", 100);

BluetoothManager::BluetoothManager()
    : _connected(false), _started(false) {
    strncpy(_device, "Esperando...", sizeof(_device) - 1);
}

void BluetoothManager::begin() {
    if (_started) return;
    s_ble.begin();
    _started = true;
    strncpy(_device, "Esperando...", sizeof(_device) - 1);
}

void BluetoothManager::stop() {
    // BleKeyboard no tiene end(), solo desconectamos lógicamente
    _connected = false;
    _started   = false;
    strncpy(_device, "Desconectado", sizeof(_device) - 1);
}

void BluetoothManager::update() {
    _checkConnection();
}

void BluetoothManager::_checkConnection() {
    bool now = s_ble.isConnected();
    if (now != _connected) {
        _connected = now;
        if (_connected) {
            strncpy(_device, "Conectado", sizeof(_device) - 1);
        } else {
            strncpy(_device, "Esperando...", sizeof(_device) - 1);
        }
    }
}

void BluetoothManager::togglePlay() {
    if (_connected) s_ble.write(KEY_MEDIA_PLAY_PAUSE);
}

void BluetoothManager::next() {
    if (_connected) s_ble.write(KEY_MEDIA_NEXT_TRACK);
}

void BluetoothManager::previous() {
    if (_connected) s_ble.write(KEY_MEDIA_PREVIOUS_TRACK);
}

void BluetoothManager::volumeUp() {
    if (_connected) s_ble.write(KEY_MEDIA_VOLUME_UP);
}

void BluetoothManager::volumeDown() {
    if (_connected) s_ble.write(KEY_MEDIA_VOLUME_DOWN);
}
