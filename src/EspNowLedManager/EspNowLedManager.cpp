#include "EspNowLedManager.h"

const uint8_t EspNowLedManager::palette[][3] = {
    {255,   0,   0},
    {  0, 255,   0},
    {  0,   0, 255},
    {128,   0, 128},
    {255, 255,   0},
    {  0, 255, 255},
    {255, 255, 255}
};
const int EspNowLedManager::COLOR_COUNT = sizeof(EspNowLedManager::palette) / sizeof(EspNowLedManager::palette[0]);

const char* EspNowLedManager::colorNames[] = {
    "Rojo", "Verde", "Azul", "Purpura", "Amarillo", "Cian", "Blanco"
};

const char* EspNowLedManager::effectNames[] = {
    "Solido", "Arcoiris", "Confeti", "Sinelon", "Juggle", "Fuego"
};
const int EspNowLedManager::EFFECT_COUNT = 6;

EspNowLedManager::EspNowLedManager(EspNowManager& manager, const uint8_t* peerMac)
    : mgr(manager) {
    memcpy(mac, peerMac, 6);
    effect     = 0;
    storedBrightness = 120;
    bps        = 10;
    r = 255; g = 0; b = 0;
    colorIndex = 0;
    ready      = false;
    on         = true;
    field      = 0;
    lastSendMs = 0;
}

void EspNowLedManager::begin() {
    if (!mgr.begin()) {
        Serial.println("[LED] mgr.begin failed");
        return;
    }
    if (!mgr.addPeer(mac)) {
        Serial.println("[LED] add peer failed");
        return;
    }
    ready = true;
    Serial.println("[LED] ready");
    send();
}

void EspNowLedManager::end() {
    if (!ready && !mgr.isActive()) return;
    ready = false;
    if (mgr.isActive()) {
        mgr.removePeer(mac);
        mgr.end();
    }
    Serial.println("[LED] ended");
}

void EspNowLedManager::fieldUp() {
    field = (field + FIELD_COUNT - 1) % FIELD_COUNT;
}

void EspNowLedManager::fieldDown() {
    field = (field + 1) % FIELD_COUNT;
}

void EspNowLedManager::fieldLeft() {
    switch (field) {
        case 0:
            colorIndex = (colorIndex + COLOR_COUNT - 1) % COLOR_COUNT;
            r = palette[colorIndex][0];
            g = palette[colorIndex][1];
            b = palette[colorIndex][2];
            break;
        case 1:
            storedBrightness = max(10, storedBrightness - 15);
            break;
        case 2:
            bps = max(1, bps - 5);
            break;
    }
}

void EspNowLedManager::fieldRight() {
    switch (field) {
        case 0:
            colorIndex = (colorIndex + 1) % COLOR_COUNT;
            r = palette[colorIndex][0];
            g = palette[colorIndex][1];
            b = palette[colorIndex][2];
            break;
        case 1:
            storedBrightness = min(255, storedBrightness + 15);
            break;
        case 2:
            bps = min(60, bps + 5);
            break;
    }
}

void EspNowLedManager::nextEffect() {
    effect = (effect + 1) % EFFECT_COUNT;
}

void EspNowLedManager::prevEffect() {
    effect = (effect + EFFECT_COUNT - 1) % EFFECT_COUNT;
}

void EspNowLedManager::toggle() {
    on = !on;
}

const char* EspNowLedManager::getEffectName() const {
    if (effect >= 0 && effect < EFFECT_COUNT)
        return effectNames[effect];
    return "?";
}

const char* EspNowLedManager::getColorName() const {
    if (colorIndex >= 0 && colorIndex < COLOR_COUNT)
        return colorNames[colorIndex];
    return "?";
}

void EspNowLedManager::send() {
    if (!ready) return;

    unsigned long now = millis();
    if (now - lastSendMs < 50) return;
    lastSendMs = now;

    struct {
        uint8_t effect;
        uint8_t brightness;
        uint8_t r, g, b;
        uint8_t bps;
    } pkt;

    pkt.effect     = effect;
    pkt.brightness = on ? storedBrightness : 0;
    pkt.r = r; pkt.g = g; pkt.b = b;
    pkt.bps        = bps;

    mgr.sendBytes(mac, (uint8_t*)&pkt, sizeof(pkt));
}
