#pragma once
#include <Arduino.h>

// =====================================================
// Contrato binario con el receptor (tira WS2812B, ESP32-C3)
// =====================================================
//
// El receptor DESCARTA cualquier paquete cuyo tamano no sea exactamente 6
// bytes. Todos los campos son uint8_t, asi que no hay padding y el layout es
// identico en cualquier compilador. No cambiar el orden.

struct LedPacket {
    uint8_t effect;      // byte 0: 0-5, fuera de rango el receptor lo ignora
    uint8_t brightness;  // byte 1: 0-255
    uint8_t r;           // byte 2: 0-255
    uint8_t g;           // byte 3: 0-255
    uint8_t b;           // byte 4: 0-255
    uint8_t bps;         // byte 5: 1-60, el receptor lo recorta
};

static_assert(sizeof(LedPacket) == 6, "LedPacket debe medir exactamente 6 bytes");

enum LedEffect : uint8_t {
    LED_FX_SOLID    = 0,
    LED_FX_RAINBOW  = 1,
    LED_FX_CONFETTI = 2,
    LED_FX_SINELON  = 3,
    LED_FX_JUGGLE   = 4,
    LED_FX_FIRE     = 5,
    LED_FX_COUNT    = 6
};

const char* ledEffectName(uint8_t effect);

// Canal del receptor. Emisor y receptor DEBEN coincidir o no llega nada,
// aunque el send_cb reporte OK (el ACK es de radio, no de aplicacion).
#define LEDSTRIP_CHANNEL 1

// =====================================================
// Manager
// =====================================================
//
// Radio ESP-NOW de solo envio para la tira LED. A diferencia de
// EspNowManager (Cam Car) no recibe nada, asi que no reserva buffer de video.
//
// IMPORTANTE: esp_now_init() es global. Este manager y EspNowManager no pueden
// estar activos a la vez; cada herramienta hace begin() al entrar y end() al
// salir, que es lo que los mantiene excluyentes.
class EspNowLedManager {
public:
    EspNowLedManager(const uint8_t mac[6], uint8_t channel = LEDSTRIP_CHANNEL);

    bool begin();   // enciende WiFi STA + ESP-NOW, fija canal y registra el peer
    void end();     // apaga ESP-NOW y WiFi por completo

    bool isActive() const { return active; }

    bool send(const LedPacket& pkt);

    // Cambiar destino en caliente (dirigido o broadcast)
    void setPeer(const uint8_t mac[6]);
    const uint8_t* getPeer() const { return peerMac; }
    bool isBroadcast() const;

    // Resultado del ultimo envio (ACK de radio, no de aplicacion)
    bool          lastSendOk() const { return sendOk; }
    unsigned long lastSendMs() const { return lastMs; }
    uint32_t      getSentCount() const { return sentCount; }
    uint32_t      getFailCount() const { return failCount; }

    // Uso interno: lo llama el callback de radio desde el .cpp. Recibe un bool
    // en vez de esp_now_send_status_t a proposito, para no arrastrar esp_now.h
    // a todo lo que incluya este header.
    void onSendResult(bool ok);

private:
    uint8_t peerMac[6];
    uint8_t channel;
    bool    active;

    volatile bool     sendOk;
    volatile uint32_t sentCount;
    volatile uint32_t failCount;
    unsigned long     lastMs;

    bool registerPeer();
};
