#include "LinternaManager.h"
#include "../GlobalConfig/GlobalConfig.h"

#define LINTERNA_NUM_LEDS   1
#define LINTERNA_LED_TYPE   WS2812B
#define LINTERNA_COLOR_ORDER GRB

static CRGB s_led[LINTERNA_NUM_LEDS];

static const CRGB LINTERNA_COLORS[LinternaManager::NUM_COLORS] = {
    CRGB::Blue,
    CRGB::White,
    CRGB::Red,
    CRGB::Green,
    CRGB::Cyan,
    CRGB::Magenta,
    CRGB::Yellow,
    CRGB(255, 128, 0),   // Naranja
};

static const char* LINTERNA_COLOR_NAMES[LinternaManager::NUM_COLORS] = {
    "Azul", "Blanco", "Rojo", "Verde",
    "Cian", "Magenta", "Amarillo", "Naranja",
};

LinternaManager::LinternaManager()
    : on(false), brightness(120), colorIndex(0)  // azul por defecto
{}

void LinternaManager::begin() {
    FastLED.addLeds<LINTERNA_LED_TYPE, LINTERNA_PIN, LINTERNA_COLOR_ORDER>(
        s_led, LINTERNA_NUM_LEDS
    );
    FastLED.setBrightness(0);
    FastLED.clear();
    FastLED.show();
}

void LinternaManager::toggle() {
    on = !on;
    apply();
}

void LinternaManager::nextColor() {
    colorIndex = (colorIndex + 1) % NUM_COLORS;
    if (on) apply();
}

void LinternaManager::prevColor() {
    colorIndex = (colorIndex + NUM_COLORS - 1) % NUM_COLORS;
    if (on) apply();
}

void LinternaManager::brightnessUp() {
    brightness = (uint8_t)min(255, (int)brightness + 25);
    if (on) apply();
}

void LinternaManager::brightnessDown() {
    brightness = (uint8_t)max(10, (int)brightness - 25);
    if (on) apply();
}

void LinternaManager::turnOff() {
    on = false;
    apply();
}

void LinternaManager::showColor(uint8_t r, uint8_t g, uint8_t b, uint8_t bright) {
    FastLED.setBrightness(bright);
    fill_solid(s_led, LINTERNA_NUM_LEDS, CRGB(r, g, b));
    FastLED.show();
}

const char* LinternaManager::getColorName() const {
    return LINTERNA_COLOR_NAMES[colorIndex];
}

void LinternaManager::apply() {
    if (on) {
        FastLED.setBrightness(brightness);
        fill_solid(s_led, LINTERNA_NUM_LEDS, LINTERNA_COLORS[colorIndex]);
    } else {
        FastLED.setBrightness(0);
        FastLED.clear();
    }
    FastLED.show();
}
