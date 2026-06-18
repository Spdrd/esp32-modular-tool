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
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            grid[y][x] = 0;
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
    grid[cursorY][cursorX] = (grid[cursorY][cursorX] == c) ? 0 : c;
}

void CanvasManager::nextColor() {
    colorIndex = (colorIndex + 1) % COLOR_COUNT;
}

uint16_t CanvasManager::getCurrentColor() const {
    return palette[colorIndex];
}
