#pragma once
#include <Arduino.h>
#include "EspNowManager/EspNowManager.h"

class EspNowLedManager {
public:
    EspNowLedManager(EspNowManager& mgr, const uint8_t* peerMac);

    void begin();
    void end();
    void send();

    void fieldUp();
    void fieldDown();
    void fieldLeft();
    void fieldRight();

    void nextEffect();
    void prevEffect();

    void toggle();
    bool isOn()  const { return on; }
    bool isReady() const { return ready; }

    int  getField()           const { return field; }
    int  getBrightness()      const { return storedBrightness; }
    int  getBps()             const { return bps; }
    uint8_t getR()            const { return r; }
    uint8_t getG()            const { return g; }
    uint8_t getB()            const { return b; }
    const char* getEffectName() const;
    const char* getColorName() const;

    static const int FIELD_COUNT = 3;

private:
    EspNowManager& mgr;
    uint8_t mac[6];
    int  effect;
    int  storedBrightness;
    int  bps;
    uint8_t r, g, b;
    int  colorIndex;
    bool ready;
    bool on;
    int  field;
    unsigned long lastSendMs;

    static const uint8_t palette[][3];
    static const char* colorNames[];
    static const int COLOR_COUNT;
    static const char* effectNames[];
    static const int EFFECT_COUNT;
};
