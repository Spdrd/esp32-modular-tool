#pragma once
#include <Arduino.h>
#include <Adafruit_GC9A01A.h>

class CanvasManager {
public:
    CanvasManager();

    void reset();
    void moveCursor(int dx, int dy);
    void paint();
    void nextColor();
    void render(Adafruit_GC9A01A& tft);

private:
    static const int COLS = 20;
    static const int ROWS = 20;
    static const int CELL = 10;
    static const int OFSX = 20;
    static const int OFSY = 30;
    static const int COLOR_COUNT = 9;

    uint16_t grid[COLS][ROWS];
    int cursorX;
    int cursorY;
    int colorIndex;

    static const uint16_t palette[COLOR_COUNT];
};
