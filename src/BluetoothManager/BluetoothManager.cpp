#include "BluetoothManager.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEHIDDevice.h>
#include <BLE2902.h>
#include <BLESecurity.h>
#include <esp_bt.h>
#include <esp_gap_ble_api.h>

BluetoothManager* BluetoothManager::instance = nullptr;

// =====================================================
// REPORT MAP HID - teclado estandar (report ID 1)
// Reporte de 8 bytes: [modificadores][reservado][6 teclas]
// Se escribe en bytes crudos a proposito: no depende de los macros de
// HIDTypes.h, que cambian de nombre entre versiones del framework.
// =====================================================

static const uint8_t HID_KEYBOARD_REPORT_MAP[] = {
    0x05, 0x01,  // Usage Page (Generic Desktop)
    0x09, 0x06,  // Usage (Keyboard)
    0xA1, 0x01,  // Collection (Application)
    0x85, 0x01,  //   Report ID (1)

    0x05, 0x07,  //   Usage Page (Keyboard/Keypad)
    0x19, 0xE0,  //   Usage Minimum (LeftControl)
    0x29, 0xE7,  //   Usage Maximum (RightGUI)
    0x15, 0x00,  //   Logical Minimum (0)
    0x25, 0x01,  //   Logical Maximum (1)
    0x75, 0x01,  //   Report Size (1)
    0x95, 0x08,  //   Report Count (8)
    0x81, 0x02,  //   Input (Data, Variable, Absolute) -> byte modificadores

    0x95, 0x01,  //   Report Count (1)
    0x75, 0x08,  //   Report Size (8)
    0x81, 0x03,  //   Input (Constant) -> byte reservado

    0x95, 0x05,  //   Report Count (5)
    0x75, 0x01,  //   Report Size (1)
    0x05, 0x08,  //   Usage Page (LEDs)
    0x19, 0x01,  //   Usage Minimum (Num Lock)
    0x29, 0x05,  //   Usage Maximum (Kana)
    0x91, 0x02,  //   Output (Data, Variable, Absolute) -> LEDs del host

    0x95, 0x01,  //   Report Count (1)
    0x75, 0x03,  //   Report Size (3)
    0x91, 0x03,  //   Output (Constant) -> relleno

    // 6 teclas simultaneas, rango COMPLETO de la pagina de teclado (0x00-0xFF).
    // Los maximos van en forma de 2 bytes (0x26 / 0x2A) a proposito: en el
    // campo de 1 byte el valor es con signo y 0xFF se leeria como -1.
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x05, 0x07,        //   Usage Page (Keyboard/Keypad)
    0x19, 0x00,        //   Usage Minimum (0)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (255)
    0x81, 0x00,        //   Input (Data, Array) -> cualquier tecla existente

    0xC0,              // End Collection

    // --- Consumer Control (report ID 2) ---
    // Un codigo de uso de 16 bits en vez de bits fijos: cubre la pagina
    // completa (0x000-0x3FF), asi que se puede mandar cualquier accion
    // multimedia, de sistema o de navegador sin volver a tocar el descriptor.
    0x05, 0x0C,        // Usage Page (Consumer)
    0x09, 0x01,        // Usage (Consumer Control)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x02,        //   Report ID (2)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
    0x19, 0x00,        //   Usage Minimum (0)
    0x2A, 0xFF, 0x03,  //   Usage Maximum (1023)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x00,        //   Input (Data, Array) -> un codigo de uso por reporte
    0xC0,              // End Collection

    // --- Mouse (report ID 3) ---
    // Payload de 5 bytes: [botones][dX][dY][rueda][pan horizontal].
    // Se incluyen los 5 botones y el pan aunque hoy no se usen todos: ampliar
    // el descriptor mas adelante obligaria a re-emparejar todos los hosts.
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x03,        //   Report ID (3)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)

    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (boton 1)
    0x29, 0x05,        //     Usage Maximum (boton 5)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x05,        //     Report Count (5)
    0x81, 0x02,        //     Input (Data, Variable, Absolute)
    0x75, 0x03,        //     Report Size (3)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x03,        //     Input (Constant) -> relleno hasta el byte

    0x05, 0x01,        //     Usage Page (Generic Desktop)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x02,        //     Report Count (2)
    0x81, 0x06,        //     Input (Data, Variable, Relative)

    0x09, 0x38,        //     Usage (Wheel)
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data, Variable, Relative)

    0x05, 0x0C,        //     Usage Page (Consumer)
    0x0A, 0x38, 0x02,  //     Usage (AC Pan) -> rueda horizontal
    0x15, 0x81,        //     Logical Minimum (-127)
    0x25, 0x7F,        //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x06,        //     Input (Data, Variable, Relative)

    0xC0,              //   End Collection
    0xC0               // End Collection
};

// =====================================================
// CALLBACKS DE CONEXION
// =====================================================

class BtServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* srv) override {
        if (BluetoothManager::instance) BluetoothManager::instance->onConnectEvent(true);
        Serial.println("[BT] host conectado");
    }
    void onDisconnect(BLEServer* srv) override {
        BluetoothManager* mgr = BluetoothManager::instance;
        if (mgr) mgr->onConnectEvent(false);

        // Este callback tambien dispara mientras end() esta apagando la radio.
        // Re-anunciar en mitad del teardown deja el stack en un estado invalido,
        // asi que solo se re-anuncia si seguimos activos de verdad.
        if (mgr && mgr->isActive()) {
            Serial.println("[BT] host desconectado, re-anunciando");
            BLEDevice::startAdvertising();
        } else {
            Serial.println("[BT] host desconectado");
        }
    }
};

static BtServerCallbacks s_serverCallbacks;

// notify() devuelve void y falla en silencio: si el host no esta suscrito o el
// enlace no esta cifrado, el reporte se descarta sin rastro. Este callback
// expone el motivo real de cada envio fallido.
class BtReportCallbacks : public BLECharacteristicCallbacks {
    void onStatus(BLECharacteristic* c, Status s, uint32_t code) override {
        switch (s) {
            case SUCCESS_NOTIFY:
                break;  // caso normal: no ensuciar el log
            case ERROR_NO_CLIENT:
                Serial.println("[BT] notify descartado: sin cliente conectado");
                break;
            case ERROR_NOTIFY_DISABLED:
                Serial.println("[BT] notify descartado: CCCD apagado (host sin suscribir)");
                break;
            case ERROR_GATT:
                Serial.print("[BT] notify fallo GATT rc=");
                Serial.print(code);
                Serial.println("  (enlace sin cifrar o handle invalido)");
                break;
            default:
                Serial.print("[BT] notify status=");
                Serial.println((int)s);
                break;
        }
    }
};

static BtReportCallbacks s_reportCallbacks;

// =====================================================
// CONSTRUCTOR
// =====================================================

BluetoothManager::BluetoothManager(BluetoothConfig config)
    : config(config), active(false), connected(false),
      server(nullptr), hid(nullptr), inputKeyboard(nullptr), inputMedia(nullptr),
      inputMouse(nullptr), mouseButtons(0) {
    memset(lastReport, 0, sizeof(lastReport));
}

void BluetoothManager::onConnectEvent(bool isConnected) {
    connected = isConnected;
    memset(lastReport, 0, sizeof(lastReport));  // arrancar siempre en limpio

    if (isConnected) enableNotifications();
}

// El CCCD (descriptor 0x2902) nace deshabilitado y notify() descarta el envio
// en silencio si sigue asi. Un host ya emparejado NO vuelve a escribirlo al
// reconectar -porque por spec el servidor deberia recordar la suscripcion- y
// Bluedroid no la persiste. Sin esto el teclado emparejado "no responde".
void BluetoothManager::enableNotifications() {
    BLECharacteristic* reports[] = { inputKeyboard, inputMedia, inputMouse };
    const char*        names[]   = { "teclado", "media", "mouse" };

    for (int i = 0; i < 3; i++) {
        if (!reports[i]) continue;
        BLE2902* cccd = (BLE2902*)reports[i]->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
        Serial.print("[BT] CCCD ");
        Serial.print(names[i]);
        if (cccd) {
            bool wasOn = cccd->getNotifications();
            cccd->setNotifications(true);
            Serial.println(wasOn ? ": ya activo" : ": forzado a activo");
        } else {
            Serial.println(": NO ENCONTRADO");
        }
    }
}

// =====================================================
// BEGIN
// =====================================================

bool BluetoothManager::begin() {
    if (active) return true;

    // Solo usamos BLE: devolver la RAM del controlador Bluetooth clasico.
    // A partir del segundo begin() ya esta liberada y retorna error benigno.
    static bool classicReleased = false;
    if (!classicReleased) {
        esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
        classicReleased = true;
    }

    instance = this;
    connected = false;
    memset(lastReport, 0, sizeof(lastReport));

    BLEDevice::init(config.deviceName);

    server = BLEDevice::createServer();
    if (!server) {
        Serial.println("[BT] createServer fallo");
        BLEDevice::deinit(true);
        return false;
    }
    server->setCallbacks(&s_serverCallbacks);

    hid = new BLEHIDDevice(server);
    inputKeyboard = hid->inputReport(1);
    hid->outputReport(1);   // el report map declara LEDs; el host escribe aqui
    inputMedia    = hid->inputReport(2);   // control multimedia
    inputMouse    = hid->inputReport(3);   // mouse

    inputKeyboard->setCallbacks(&s_reportCallbacks);
    inputMedia->setCallbacks(&s_reportCallbacks);
    inputMouse->setCallbacks(&s_reportCallbacks);

    // Ojo: manufacturer() sin argumentos NO es un getter, es quien crea la
    // caracteristica. El constructor de BLEHIDDevice no la crea, asi que
    // llamar directo a manufacturer(std::string) revienta con LoadProhibited.
    hid->manufacturer()->setValue(std::string(config.manufacturer));
    hid->pnp(0x02, config.vid, config.pid, config.version);
    hid->hidInfo(0x00, 0x01);   // country = 0, flags = remote wake
    hid->reportMap((uint8_t*)HID_KEYBOARD_REPORT_MAP, sizeof(HID_KEYBOARD_REPORT_MAP));
    hid->startServices();
    hid->setBatteryLevel(100);

    // Emparejamiento sin PIN (el dispositivo no tiene teclado numerico)
    // En la pila a proposito: sus setters solo empujan parametros al stack BLE
    // y el destructor esta vacio, asi que con 'new' se filtraria en cada begin().
    BLESecurity security;
    security.setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    security.setCapability(ESP_IO_CAP_NONE);
    security.setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

    BLEAdvertising* adv = server->getAdvertising();
    adv->setAppearance(HID_KEYBOARD);
    adv->addServiceUUID(hid->hidService()->getUUID());
    adv->setScanResponse(true);
    adv->start();

    active = true;
    Serial.print("[BT] activo como \"");
    Serial.print(config.deviceName);
    Serial.println("\", esperando emparejamiento");
    return true;
}

// =====================================================
// END
// =====================================================

void BluetoothManager::end() {
    if (!active) return;

    keyReleaseAll();

    // Bajar la bandera ANTES del teardown: el onDisconnect que dispara el
    // apagado la consulta para no re-anunciar sobre un stack a medio cerrar.
    active = false;

    BLEDevice::stopAdvertising();

    // deinit(false), NO deinit(true). La libreria solo limpia su flag interno
    // 'initialized' en la rama del false; con true libera la memoria del
    // controlador de forma irreversible y deja el flag encendido, asi que el
    // siguiente init() se salta toda la inicializacion y congela el chip.
    // Con false la radio igual queda apagada (controller disable + deinit),
    // que es lo que interesa para la bateria.
    BLEDevice::deinit(false);

    // El destructor de BLEHIDDevice esta vacio: solo libera el envoltorio.
    // Los servicios y caracteristicas los sigue reteniendo BLEServer y la
    // libreria no ofrece forma de destruirlos, asi que ese resto se filtra.
    delete hid;

    server        = nullptr;
    hid           = nullptr;
    inputKeyboard = nullptr;
    inputMedia    = nullptr;
    inputMouse    = nullptr;
    mouseButtons  = 0;
    instance      = nullptr;
    connected     = false;
    active        = false;

    Serial.println("[BT] apagado");
}

// =====================================================
// HID - TECLADO
// =====================================================

void BluetoothManager::sendRaw(const uint8_t* report8) {
    if (!active || !connected || !inputKeyboard) return;
    if (memcmp(report8, lastReport, 8) == 0) return;  // nada cambio

    memcpy(lastReport, report8, 8);
    inputKeyboard->setValue((uint8_t*)report8, 8);
    inputKeyboard->notify();
}

void BluetoothManager::keyboardReport(uint8_t modifiers, const uint8_t* keys, uint8_t numKeys) {
    uint8_t report[8] = {0};
    report[0] = modifiers;
    // report[1] queda reservado en 0
    if (numKeys > 6) numKeys = 6;
    for (uint8_t i = 0; i < numKeys; i++) report[2 + i] = keys[i];
    sendRaw(report);
}

void BluetoothManager::keyTap(uint8_t keycode, uint8_t modifiers) {
    uint8_t one = keycode;
    keyboardReport(modifiers, &one, 1);
    delay(12);
    keyReleaseAll();
}

void BluetoothManager::keyReleaseAll() {
    uint8_t empty[8] = {0};
    sendRaw(empty);
}

// =====================================================
// VINCULOS (BONDS)
// =====================================================

int BluetoothManager::getBondedCount() const {
    if (!active) return -1;   // sin radio encendida no se puede consultar
    return esp_ble_get_bond_device_num();
}

int BluetoothManager::clearBonds() {
    if (!active) return -1;

    int num = esp_ble_get_bond_device_num();
    if (num <= 0) return 0;

    esp_ble_bond_dev_t* list =
        (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * num);
    if (!list) return -1;

    int removed = 0;
    if (esp_ble_get_bond_device_list(&num, list) == ESP_OK) {
        for (int i = 0; i < num; i++) {
            if (esp_ble_remove_bond_device(list[i].bd_addr) == ESP_OK) removed++;
        }
    }
    free(list);

    Serial.print("[BT] vinculos borrados: ");
    Serial.println(removed);
    return removed;
}

// =====================================================
// HID - CONSUMER CONTROL
// =====================================================

// =====================================================
// HID - MOUSE
// =====================================================

static int8_t clampDelta(int v) {
    if (v >  127) return  127;
    if (v < -127) return -127;
    return (int8_t)v;
}

void BluetoothManager::mouseReport(uint8_t buttons, int dx, int dy, int wheel, int pan) {
    if (!active || !connected || !inputMouse) return;

    mouseButtons = buttons;

    uint8_t rep[5];
    rep[0] = buttons;
    rep[1] = (uint8_t)clampDelta(dx);
    rep[2] = (uint8_t)clampDelta(dy);
    rep[3] = (uint8_t)clampDelta(wheel);
    rep[4] = (uint8_t)clampDelta(pan);

    // Sin deduplicacion: los movimientos son relativos, asi que dos reportes
    // iguales seguidos son dos desplazamientos distintos, no una repeticion.
    inputMouse->setValue(rep, 5);
    inputMouse->notify();
}

void BluetoothManager::mouseMove(int dx, int dy) {
    mouseReport(mouseButtons, dx, dy, 0, 0);
}

void BluetoothManager::mouseScroll(int wheel, int pan) {
    mouseReport(mouseButtons, 0, 0, wheel, pan);
}

void BluetoothManager::mouseSetButtons(uint8_t buttons) {
    mouseReport(buttons, 0, 0, 0, 0);
}

void BluetoothManager::consumerTap(uint16_t usage) {
    if (!active || !connected || !inputMedia) return;

    // Codigo de uso de 16 bits, little endian
    uint8_t rep[2] = { (uint8_t)(usage & 0xFF), (uint8_t)(usage >> 8) };
    inputMedia->setValue(rep, 2);
    inputMedia->notify();

    delay(12);

    rep[0] = 0; rep[1] = 0;     // soltar, si no el host repite la accion
    inputMedia->setValue(rep, 2);
    inputMedia->notify();
}
