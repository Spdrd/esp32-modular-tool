#include "EspNowLedManager.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Puente entre el callback C de la radio y la instancia. Vive aqui y no en la
// clase para que el header no necesite los tipos de esp_now.
static EspNowLedManager* s_instance = nullptr;

static void espNowLedSendCb(const uint8_t* mac, esp_now_send_status_t status) {
    if (s_instance) s_instance->onSendResult(status == ESP_NOW_SEND_SUCCESS);
}

static const char* LED_FX_NAMES[LED_FX_COUNT] = {
    "SOLID", "RAINBOW", "CONFETTI", "SINELON", "JUGGLE", "FIRE"
};

const char* ledEffectName(uint8_t effect) {
    return (effect < LED_FX_COUNT) ? LED_FX_NAMES[effect] : "?";
}

EspNowLedManager::EspNowLedManager(const uint8_t mac[6], uint8_t channel)
    : channel(channel), active(false), sendOk(false),
      sentCount(0), failCount(0), lastMs(0) {
    memcpy(peerMac, mac, 6);
}

void EspNowLedManager::onSendResult(bool ok) {
    sendOk = ok;
    if (ok) sentCount++;
    else    failCount++;
}

bool EspNowLedManager::isBroadcast() const {
    for (int i = 0; i < 6; i++)
        if (peerMac[i] != 0xFF) return false;
    return true;
}

bool EspNowLedManager::registerPeer() {
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, peerMac, 6);
    peer.channel = channel;      // debe coincidir con el receptor
    peer.encrypt = false;        // el receptor no tiene claves configuradas
    peer.ifidx   = WIFI_IF_STA;

    if (esp_now_is_peer_exist(peerMac)) return true;

    esp_err_t rc = esp_now_add_peer(&peer);
    if (rc != ESP_OK) {
        Serial.print("[LED] add_peer fallo rc=");
        Serial.println(rc);
        return false;
    }
    return true;
}

bool EspNowLedManager::begin() {
    if (active) return true;

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
        Serial.println("[LED] esp_now_init fallo");
        WiFi.mode(WIFI_OFF);
        return false;
    }

    // El canal se fija DESPUES de levantar el STA: WiFi.disconnect() puede
    // devolverlo al valor por defecto. Sin esto el send_cb reporta OK pero
    // el receptor nunca escucha el paquete.
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

    s_instance = this;
    esp_now_register_send_cb(espNowLedSendCb);

    if (!registerPeer()) {
        esp_now_deinit();
        WiFi.mode(WIFI_OFF);
        s_instance = nullptr;
        return false;
    }

    sendOk    = false;
    sentCount = 0;
    failCount = 0;
    active    = true;

    Serial.print("[LED] activo en canal ");
    Serial.print(channel);
    Serial.println(isBroadcast() ? " (broadcast)" : " (dirigido)");
    return true;
}

void EspNowLedManager::end() {
    if (!active) return;

    active = false;
    esp_now_unregister_send_cb();
    esp_now_deinit();
    WiFi.mode(WIFI_OFF);
    s_instance = nullptr;

    Serial.println("[LED] apagado");
}

void EspNowLedManager::setPeer(const uint8_t mac[6]) {
    if (active && esp_now_is_peer_exist(peerMac)) esp_now_del_peer(peerMac);
    memcpy(peerMac, mac, 6);
    if (active) registerPeer();
}

bool EspNowLedManager::send(const LedPacket& pkt) {
    if (!active) return false;

    lastMs = millis();
    esp_err_t rc = esp_now_send(peerMac, (const uint8_t*)&pkt, sizeof(pkt));
    if (rc != ESP_OK) {
        failCount++;
        sendOk = false;
        Serial.print("[LED] esp_now_send rc=");
        Serial.println(rc);
        return false;
    }
    return true;   // el exito real llega por el send_cb
}
