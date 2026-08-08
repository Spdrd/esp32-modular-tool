#pragma once
#include <Arduino.h>
#include "HidKeys.h"

class BLEServer;
class BLEHIDDevice;
class BLECharacteristic;

struct BluetoothConfig {
    const char* deviceName;   // nombre visible al emparejar
    const char* manufacturer;
    uint16_t    vid;
    uint16_t    pid;
    uint16_t    version;
};

// Maneja la radio Bluetooth LE bajo demanda: begin()/end() encienden y apagan
// el stack completo para no gastar bateria cuando no se usa (mismo criterio
// que EspNowManager).
//
// De momento expone un perfil HID de teclado. La clase esta pensada para
// crecer: getServer()/getHid() permiten colgar servicios adicionales
// (mouse, media keys, gamepad, UART...) sin reescribir el ciclo de vida.
class BluetoothManager {
public:
    BluetoothManager(BluetoothConfig config);

    // --- Ciclo de vida de la radio ---
    bool begin();    // enciende BLE, publica el servicio HID y anuncia. true si OK
    void end();      // suelta todo y apaga el stack BLE

    bool isActive()    const { return active; }
    bool isConnected() const { return connected; }

    const char* getDeviceName() const { return config.deviceName; }

    // --- Perfil HID: teclado ---
    // Reporte crudo: modificadores + hasta 6 teclas simultaneas.
    // Pasar numKeys = 0 equivale a "ninguna tecla pulsada".
    void keyboardReport(uint8_t modifiers, const uint8_t* keys, uint8_t numKeys);

    // Pulsa y suelta una tecla (para acciones puntuales).
    void keyTap(uint8_t keycode, uint8_t modifiers = HID_MOD_NONE);

    // Suelta todo lo que estuviera pulsado.
    void keyReleaseAll();

    // --- Perfil HID: Consumer Control ---
    // Recibe un codigo de uso de la pagina Consumer (constantes HID_CC_*, o
    // cualquier valor 0x000-0x3FF). Es una accion puntual: pulsa y suelta.
    void consumerTap(uint16_t usage);

    // --- Vinculos (bonds) ---
    // Un host emparejado cachea la tabla de servicios y el descriptor HID, y
    // no los relee al reconectar. Si cambia lo que ofrece el dispositivo, hay
    // que romper el vinculo por ambos lados para que lo redescubra.
    // Requieren la radio encendida (begin() previo).
    int  getBondedCount() const;
    int  clearBonds();          // borra todos; retorna cuantos elimino

    // --- Acceso para futuros perfiles ---
    BLEServer*    getServer() const { return server; }
    BLEHIDDevice* getHid()    const { return hid; }

private:
    BluetoothConfig config;

    bool          active;
    volatile bool connected;

    BLEServer*         server;
    BLEHIDDevice*      hid;
    BLECharacteristic* inputKeyboard;
    BLECharacteristic* inputMedia;

    // Estado del ultimo reporte enviado, para no repetir envios identicos
    uint8_t lastReport[8];

    void sendRaw(const uint8_t* report8);
    void enableNotifications();

    // Puente entre los callbacks C++ de BLE y esta instancia
    static BluetoothManager* instance;
    friend class BtServerCallbacks;
    void onConnectEvent(bool isConnected);
};
