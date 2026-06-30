#include "EspNowManager.h"
#include <WiFi.h>
#include <esp_now.h>

EspNowManager* EspNowManager::instance = nullptr;

EspNowManager::EspNowManager(EspNowPeerConfig config)
    : config(config), active(false), frameLen(0), frameReady(false),
      framesReceived(0), lastRxMs(0) {}

void EspNowManager::onRecvStatic(const uint8_t* mac, const uint8_t* data, int len) {
    if (instance) instance->onPacket(data, len);
}

void EspNowManager::onPacket(const uint8_t* data, int len) {
    lastRxMs = millis();

    // Fin de frame: 4 bytes exactos = marcador
    if (len == 4 &&
        data[0] == FRAME_END_MARKER[0] && data[1] == FRAME_END_MARKER[1] &&
        data[2] == FRAME_END_MARKER[2] && data[3] == FRAME_END_MARKER[3]) {
        if (frameLen > 0) {
            frameReady = true;
            framesReceived++;
        }
        return;
    }

    // Chunk de imagen: acumular si hay espacio
    if (len > 0 && frameLen + (size_t)len < FRAME_BUF_SIZE) {
        memcpy(frameBuf + frameLen, data, len);
        frameLen += len;
    }
}

bool EspNowManager::begin() {
    if (active) return true;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESPNOW] init fallo");
        WiFi.mode(WIFI_OFF);
        return false;
    }

    instance = this;
    esp_now_register_recv_cb(onRecvStatic);

    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, config.mac, 6);
    peer.channel = 0;
    peer.encrypt = false;
    if (!esp_now_is_peer_exist(config.mac)) {
        esp_now_add_peer(&peer);
    }

    frameLen   = 0;
    frameReady = false;
    active     = true;
    return true;
}

void EspNowManager::end() {
    if (!active) return;
    esp_now_unregister_recv_cb();
    esp_now_deinit();
    WiFi.mode(WIFI_OFF);
    instance   = nullptr;
    active     = false;
    frameReady = false;
    frameLen   = 0;
}

void EspNowManager::sendCommand(CommandType cmd) {
    if (!active) return;
    ControlPacket pkt;
    pkt.magic   = CONTROL_MAGIC;
    pkt.command = cmd;
    pkt.speed   = 255;
    esp_now_send(config.mac, (uint8_t*)&pkt, sizeof(pkt));
}

void EspNowManager::clearFrame() {
    frameLen   = 0;
    frameReady = false;
}
