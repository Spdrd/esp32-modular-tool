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

    // --- Perfil HID: mouse ---
    // dx/dy/wheel/pan son RELATIVOS y se recortan a [-127,127].
    // buttons es una mascara HID_MOUSE_*.
    void mouseReport(uint8_t buttons, int dx, int dy, int wheel = 0, int pan = 0);
    void mouseMove(int dx, int dy);                 // conserva los botones pulsados
    void mouseScroll(int wheel, int pan = 0);
    void mouseSetButtons(uint8_t buttons);          // pulsar/soltar sin mover
    uint8_t getMouseButtons() const { return mouseButtons; }

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
    BLECharacteristic* inputMouse;

    uint8_t mouseButtons;   // estado sostenido de los botones del mouse

    // Estado del ultimo reporte enviado, para no repetir envios identicos
    uint8_t lastReport[8];

    void sendRaw(const uint8_t* report8);
    void enableNotifications();

    // Puente entre los callbacks C++ de BLE y esta instancia
    static BluetoothManager* instance;
    friend class BtServerCallbacks;
    void onConnectEvent(bool isConnected);
};
