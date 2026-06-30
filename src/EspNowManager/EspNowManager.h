#pragma once
#include <Arduino.h>
#include "../Protocol/Protocol.h"

struct EspNowPeerConfig {
    uint8_t mac[6];
};

// Maneja la radio ESP-NOW bajo demanda: begin()/end() prenden y apagan
// WiFi explicitamente para no consumir bateria cuando no se usa.
class EspNowManager {
public:
    EspNowManager(EspNowPeerConfig config);

    bool begin();   // enciende WiFi + ESP-NOW + registra peer. true si OK
    void end();     // apaga ESP-NOW + WiFi por completo

    bool isActive() const { return active; }

    void sendCommand(CommandType cmd);

    // Video recibido del vehiculo (frame JPEG acumulado)
    bool           hasFrame()      const { return frameReady; }
    const uint8_t* getFrameBuffer() const { return frameBuf; }
    size_t         getFrameSize()   const { return frameLen; }
    void           clearFrame();

    // Estadisticas simples
    uint32_t getFramesReceived() const { return framesReceived; }
    unsigned long getLastRxMs()  const { return lastRxMs; }

private:
    EspNowPeerConfig config;
    bool active;

    static const size_t FRAME_BUF_SIZE = 24000;
    uint8_t frameBuf[FRAME_BUF_SIZE];
    size_t  frameLen;
    volatile bool frameReady;
    uint32_t framesReceived;
    unsigned long lastRxMs;

    void onPacket(const uint8_t* data, int len);

    static EspNowManager* instance;
    static void onRecvStatic(const uint8_t* mac, const uint8_t* data, int len);
};
