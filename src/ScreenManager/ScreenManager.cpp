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