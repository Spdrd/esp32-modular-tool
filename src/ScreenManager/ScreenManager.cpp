// ScreenManager.cpp

#include "ScreenManager.h"

static ScreenManager* screenInstance = nullptr;

// =====================================================
// CALLBACK JPEG
// =====================================================

bool tftOutput(
    int16_t x,
    int16_t y,
    uint16_t w,
    uint16_t h,
    uint16_t* bitmap
) {

    if (screenInstance == nullptr) {
        return false;
    }

    screenInstance->tft.drawRGBBitmap(
        x,
        y,
        bitmap,
        w,
        h
    );

    return true;
}

// =====================================================
// CONSTRUCTOR
// =====================================================

ScreenManager::ScreenManager(ScreenPinConfig cfg)
    : config(cfg),
      tft(cfg.tftCsPin, cfg.tftDcPin, cfg.tftRstPin)
{
}

// =====================================================
// BEGIN
// =====================================================

void ScreenManager::begin() {

    Serial.println("Iniciando pantalla...");

    SPI.begin(
        config.tftSclkPin,
        -1,
        config.tftMosiPin,
        config.tftCsPin
    );

    tft.begin();

    tft.setRotation(1);

    tft.fillScreen(GC9A01A_BLACK);

    tft.setCursor(60, 110);
    tft.setTextColor(GC9A01A_GREEN);
    tft.setTextSize(2);

    tft.println("MINIMAL OK");

    // =====================================
    // JPEG DECODER
    // =====================================

    screenInstance = this;

    TJpgDec.setCallback(tftOutput);

    // Opcional:
    TJpgDec.setJpgScale(1);

    Serial.println("JPEG decoder listo");
}

// =====================================================
// UPDATE
// =====================================================

void ScreenManager::update(String text) {

    tft.fillScreen(GC9A01A_BLACK);

    tft.setCursor(10, 110);

    tft.setTextColor(GC9A01A_WHITE);

    tft.setTextSize(2);

    tft.println(text);
}

// =====================================================
// CLEAR
// =====================================================

void ScreenManager::clear() {

    tft.fillScreen(GC9A01A_BLACK);
}

// =====================================================
// SHOW JPEG
// =====================================================

void ScreenManager::showJpeg(
    uint8_t* jpgBuffer,
    uint32_t jpgSize,
    int jpg_w,
    int jpg_h
) {

    if (jpgBuffer == nullptr || jpgSize == 0) {

        Serial.println("JPEG invalido");

        return;
    }

    // =========================================
    // CENTRADO AUTOMATICO
    // =========================================

    int posX = (tft.width()  - jpg_w) / 2;
    int posY = (tft.height() - jpg_h) / 2;

    // Evita coordenadas negativas
    if (posX < 0) posX = 0;
    if (posY < 0) posY = 0;

    // =========================================
    // LIMPIAR PANTALLA
    // =========================================

    tft.fillScreen(GC9A01A_BLACK);

    // =========================================
    // DIBUJAR JPEG
    // =========================================

    TJpgDec.drawJpg(
        posX,
        posY,
        jpgBuffer,
        jpgSize
    );

    // =========================================
    // DEBUG
    // =========================================

    Serial.println("JPEG mostrado");

    Serial.print("Width: ");
    Serial.println(jpg_w);

    Serial.print("Height: ");
    Serial.println(jpg_h);

    Serial.print("X: ");
    Serial.println(posX);

    Serial.print("Y: ");
    Serial.println(posY);
}

// =====================================================
// SHOW MENU
// =====================================================

void ScreenManager::showMenu(const char* title, const char* names[], int count, int selectedIndex) {
    tft.fillScreen(GC9A01A_BLACK);

    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(2);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 8);
    tft.println(title);

    tft.drawFastHLine(0, 28, tft.width(), GC9A01A_DARKGREY);

    tft.setTextSize(1);
    int startY = 36;
    for (int i = 0; i < count; i++) {
        int y = startY + i * 13;
        if (y > tft.height() - 8) break;

        tft.getTextBounds(names[i], 0, 0, &x1, &y1, &w, &h);
        int cx = (tft.width() - w) / 2;

        if (i == selectedIndex) {
            tft.fillRect(cx - 3, y - 1, w + 6, 11, GC9A01A_BLUE);
            tft.setTextColor(GC9A01A_WHITE);
            tft.setCursor(cx, y);
        } else {
            tft.setTextColor(GC9A01A_LIGHTGREY);
            tft.setCursor(cx, y);
        }
        tft.println(names[i]);
    }
}

// =====================================================
// DRAW SNAKE
// =====================================================

void ScreenManager::drawSnake(const int bodyX[], const int bodyY[], int length,
                              int foodX, int foodY, int score, bool gameOver) {
    const int COLS = 22, ROWS = 16, CELL = 10, OFSX = 10, OFSY = 25;

    tft.fillScreen(GC9A01A_BLACK);

    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(1);
    tft.setCursor(OFSX, 8);
    tft.print("Score: ");
    tft.println(score);

    tft.drawRect(OFSX, OFSY, COLS * CELL, ROWS * CELL, GC9A01A_DARKGREY);

    for (int i = 0; i < length; i++) {
        uint16_t c = (i == 0) ? GC9A01A_GREEN : GC9A01A_GREENYELLOW;
        tft.fillRect(OFSX + bodyX[i] * CELL + 1, OFSY + bodyY[i] * CELL + 1, CELL - 2, CELL - 2, c);
    }

    tft.fillRect(OFSX + foodX * CELL + 1, OFSY + foodY * CELL + 1, CELL - 2, CELL - 2, GC9A01A_RED);

    if (gameOver) {
        tft.setTextColor(GC9A01A_RED);
        tft.setTextSize(2);
        const char* msg = "GAME OVER";
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, tft.height() / 2 - 10);
        tft.println(msg);
    }
}

// =====================================================
// DRAW CRONOMETRO
// =====================================================

void ScreenManager::drawCronometro(unsigned long elapsed, bool running,
                                   const unsigned long laps[], int lapCount) {
    tft.fillScreen(GC9A01A_BLACK);

    char buf[16];
    unsigned long t = elapsed;
    int m = t / 60000;
    int s = (t % 60000) / 1000;
    int c = (t % 1000) / 10;
    sprintf(buf, "%02d:%02d.%02d", m, s, c);

    tft.setTextSize(3);
    tft.setTextColor(GC9A01A_GREEN);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 35);
    tft.println(buf);

    tft.drawFastHLine(10, 70, 220, GC9A01A_DARKGREY);

    tft.setTextSize(1);
    tft.setCursor(10, 80);
    tft.setTextColor(running ? GC9A01A_GREEN : GC9A01A_YELLOW);
    tft.println(running ? "CORRIENDO" : "DETENIDO");

    int lapY = 95;
    int start = lapCount > 6 ? lapCount - 6 : 0;
    for (int i = start; i < lapCount; i++) {
        unsigned long lt = laps[i];
        int lm = lt / 60000;
        int ls = (lt % 60000) / 1000;
        int lc = (lt % 1000) / 10;
        sprintf(buf, "%02d:%02d.%02d", lm, ls, lc);
        tft.setCursor(10, lapY);
        tft.setTextColor(GC9A01A_CYAN);
        tft.print("V");
        tft.print(i + 1);
        tft.print(": ");
        tft.println(buf);
        lapY += 12;
    }

    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(10, 220);
    tft.print("[OK] ");
    tft.print(running ? "Pausa" : "Inicia");
    tft.print("  [A] Marca  [B] Reset");
}

// =====================================================
// DRAW DICE
// =====================================================

void ScreenManager::drawDice(int maxValue, int result, bool rolled) {
    tft.fillScreen(GC9A01A_BLACK);

    char buf[16];
    sprintf(buf, "DADO (d%d)", maxValue);

    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 15);
    tft.println(buf);

    tft.drawRoundRect(50, 50, 140, 100, 12, GC9A01A_DARKGREY);

    if (rolled) {
        sprintf(buf, "%d", result);
        tft.setTextSize(6);
        tft.setTextColor(GC9A01A_YELLOW);
        tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, 70);
        tft.println(buf);
    } else {
        tft.setTextSize(4);
        tft.setTextColor(GC9A01A_DARKGREY);
        tft.getTextBounds("?", 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, 75);
        tft.println("?");
    }

    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(10, 220);
    tft.println("[UP/DN] Caras  [OK] Tirar");
}

// =====================================================
// DRAW CANVAS
// =====================================================

void ScreenManager::drawCanvas(const uint16_t grid[], int cols, int rows,
                               int cursorX, int cursorY, uint16_t currentColor) {
    const int CELL = 10, OFSX = 20, OFSY = 30;

    tft.fillScreen(GC9A01A_BLACK);

    for (int x = 0; x < cols; x++) {
        for (int y = 0; y < rows; y++) {
            uint16_t c = grid[y * cols + x];
            if (c != 0) {
                tft.fillRect(OFSX + x * CELL, OFSY + y * CELL, CELL, CELL, c);
            }
        }
    }

    for (int x = 0; x <= cols; x++)
        tft.drawFastVLine(OFSX + x * CELL, OFSY, rows * CELL, GC9A01A_DARKGREY);
    for (int y = 0; y <= rows; y++)
        tft.drawFastHLine(OFSX, OFSY + y * CELL, cols * CELL, GC9A01A_DARKGREY);

    int px = OFSX + cursorX * CELL;
    int py = OFSY + cursorY * CELL;
    tft.drawRect(px, py, CELL, CELL, GC9A01A_YELLOW);
    tft.drawRect(px + 1, py + 1, CELL - 2, CELL - 2, GC9A01A_YELLOW);

    tft.fillRect(10, 215, 20, 20, currentColor);
    tft.drawRect(10, 215, 20, 20, GC9A01A_WHITE);

    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(40, 220);
    tft.print("[OK] Pintar  [A] Color  [B] Limpiar");
}

// =====================================================
// SHOW TEXT LINES
// =====================================================

void ScreenManager::showTextLines(
    String line1,
    String line2,
    String line3,
    uint16_t textColor
) {

    // =========================================
    // LIMPIAR
    // =========================================

    tft.fillScreen(GC9A01A_BLACK);

    // =========================================
    // CONFIG TEXTO
    // =========================================

    tft.setTextColor(textColor);

    tft.setTextSize(2);

    // =========================================
    // CENTRADO SIMPLE
    // =========================================

    int centerX = tft.width() / 2;

    int y1 = 80;
    int y2 = 120;
    int y3 = 160;

    // =========================================
    // LINEA 1
    // =========================================

    int16_t x1, y1_tmp;
    uint16_t w1, h1;

    tft.getTextBounds(
        line1,
        0,
        0,
        &x1,
        &y1_tmp,
        &w1,
        &h1
    );

    tft.setCursor(centerX - (w1 / 2), y1);

    tft.println(line1);

    // =========================================
    // LINEA 2
    // =========================================

    int16_t x2, y2_tmp;
    uint16_t w2, h2;

    tft.getTextBounds(
        line2,
        0,
        0,
        &x2,
        &y2_tmp,
        &w2,
        &h2
    );

    tft.setCursor(centerX - (w2 / 2), y2);

    tft.println(line2);

    // =========================================
    // LINEA 3
    // =========================================

    int16_t x3, y3_tmp;
    uint16_t w3, h3;

    tft.getTextBounds(
        line3,
        0,
        0,
        &x3,
        &y3_tmp,
        &w3,
        &h3
    );

    tft.setCursor(centerX - (w3 / 2), y3);

    tft.println(line3);
}