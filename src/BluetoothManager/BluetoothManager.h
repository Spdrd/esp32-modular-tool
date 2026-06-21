#pragma once
#include <Arduino.h>

class BluetoothManager {
public:
    BluetoothManager();

    void begin();   // inicia BLE HID
    void stop();    // desconecta y libera BLE
    void update();  // llamar en el loop

    void togglePlay();
    void next();
    void previous();
    void volumeUp();
    void volumeDown();

    bool        isConnected() const { return _connected; }
    const char* getDevice()   const { return _device; }

private:
    bool _connected;
    bool _started;
    char _device[32];

    void _checkConnection();
};
