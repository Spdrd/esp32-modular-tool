#pragma once
#include <FastLED.h>

// LINTERNA_PIN se define en GlobalConfig.h

class LinternaManager {
public:
    static const int NUM_COLORS = 8;

    LinternaManager();
    void begin();

    void toggle();
    void nextColor();
    void prevColor();
    void brightnessUp();
    void brightnessDown();
    void turnOff();

    bool    isOn()          const { return on; }
    uint8_t getBrightness() const { return brightness; }
    uint8_t getColorIndex() const { return colorIndex; }
    const char* getColorName() const;

private:
    bool    on;
    uint8_t brightness;
    uint8_t colorIndex;

    void apply();
};
