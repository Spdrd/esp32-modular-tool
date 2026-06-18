// ScreenManager.h

#pragma once
#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <TJpg_Decoder.h>

struct ScreenPinConfig
{
    int tftCsPin;
    int tftRstPin;
    int tftMisoPin;
    int tftDcPin;
    int tftMosiPin;
    int tftSclkPin;
};

class ScreenManager {

public:

    ScreenManager(ScreenPinConfig config);

    void begin();

    void update(String text);

    void clear();

    void showJpeg(uint8_t* jpgBuffer, uint32_t jpgSize, int jpg_w, int jpg_h);

    void showTextLines(
        String line1,
        String line2,
        String line3,
        uint16_t textColor = GC9A01A_WHITE
    );

    void showMenu(const char* title, const char* names[], int count, int selectedIndex);

    // Necesario para callback JPEG
    Adafruit_GC9A01A tft;

private:

    ScreenPinConfig config;
};