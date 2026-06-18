#include "CanvasManager.h"

const uint16_t CanvasManager::palette[COLOR_COUNT] = {
    GC9A01A_WHITE,
    GC9A01A_RED,
    GC9A01A_ORANGE,
    GC9A01A_YELLOW,
    GC9A01A_GREEN,
    GC9A01A_CYAN,
    GC9A01A_BLUE,
    GC9A01A_PURPLE,
    GC9A01A_PINK
};

CanvasManager::CanvasManager() {
    reset();
}

void CanvasManager::reset() {
    for (int x = 0; x < COLS; x++)
        for (int y = 0; y < ROWS; y++)
            grid[x][y] = 0;
    cursorX = COLS / 2;
    cursorY = ROWS / 2;
    colorIndex = 0;
}

void CanvasManager::moveCursor(int dx, int dy) {
    int nx = cursorX + dx;
    int ny = cursorY + dy;
    if (nx >= 0 && nx < COLS) cursorX = nx;
    if (ny >= 0 && ny < ROWS) cursorY = ny;
}

void CanvasManager::paint() {
    uint16_t c = palette[colorIndex];
    grid[cursorX][cursorY] = (grid[cursorX][cursorY] == c) ? 0 : c;
}

void CanvasManager::nextColor() {
    colorIndex = (colorIndex + 1) % COLOR_COUNT;
}

void CanvasManager::render(Adafruit_GC9A01A& tft) {
    tft.fillScreen(GC9A01A_BLACK);

    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if (grid[x][y] != 0) {
                tft.fillRect(OFSX + x * CELL, OFSY + y * CELL, CELL, CELL, grid[x][y]);
            }
        }
    }

    for (int x = 0; x <= COLS; x++)
        tft.drawFastVLine(OFSX + x * CELL, OFSY, ROWS * CELL, GC9A01A_DARKGREY);
    for (int y = 0; y <= ROWS; y++)
        tft.drawFastHLine(OFSX, OFSY + y * CELL, COLS * CELL, GC9A01A_DARKGREY);

    int px = OFSX + cursorX * CELL;
    int py = OFSY + cursorY * CELL;
    tft.drawRect(px, py, CELL, CELL, GC9A01A_YELLOW);
    tft.drawRect(px + 1, py + 1, CELL - 2, CELL - 2, GC9A01A_YELLOW);

    uint16_t curColor = palette[colorIndex];
    tft.fillRect(10, 215, 20, 20, curColor);
    tft.drawRect(10, 215, 20, 20, GC9A01A_WHITE);

    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(40, 220);
    tft.print("[OK] Pintar  [A] Color  [B] Limpiar");
}
