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

    int getCursorX() const { return cursorX; }
    int getCursorY() const { return cursorY; }
    uint16_t getCurrentColor() const;
    int getCols() const { return COLS; }
    int getRows() const { return ROWS; }
    const uint16_t* getGrid() const { return &grid[0][0]; }

private:
    static const int COLS = 20;
    static const int ROWS = 20;
    static const int CELL = 10;
    static const int OFSX = 20;
    static const int OFSY = 30;
    static const int COLOR_COUNT = 9;

    uint16_t grid[ROWS][COLS];
    int cursorX;
    int cursorY;
    int colorIndex;

    static const uint16_t palette[COLOR_COUNT];
};
