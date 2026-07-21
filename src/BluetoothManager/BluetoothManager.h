#pragma once
#include <Arduino.h>

class BluetoothManager {
public:
    BluetoothManager(const char* deviceName = "ESP32-Modular");

    bool begin();
    void end();
    bool isActive() const { return active; }
    bool isConnected() const;

    void send(const char* text);
    int  available();
    int  read();

    const char* getStatus() const;
    const char* getDeviceName() const { return deviceName; }

private:
    bool active;
    char deviceName[32];
    mutable char statusBuf[24];
};
