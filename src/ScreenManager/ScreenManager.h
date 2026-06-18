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

    void drawSnake(const int bodyX[], const int bodyY[], int length,
                   int foodX, int foodY, int score, bool gameOver);

    void drawCronometro(unsigned long elapsed, bool running,
                        const unsigned long laps[], int lapCount);

    void drawDice(int maxValue, int result, bool rolled);

    void drawCanvas(const uint16_t grid[], int cols, int rows,
                    int cursorX, int cursorY, uint16_t currentColor);

    void drawTimer(int minutes, int seconds, int state, int field, bool flashOn);

    void drawSimon(int highlight, int score, int state, bool flashOn);

    Adafruit_GC9A01A tft;

private:

    ScreenPinConfig config;
};
