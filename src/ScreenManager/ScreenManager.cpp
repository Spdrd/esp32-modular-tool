// ScreenManager.cpp

#include "ScreenManager.h"
#include <math.h>

static ScreenManager* screenInstance = nullptr;

// Contadores para deteccion de frame negro (reiniciados antes de cada decode)
static uint32_t s_jpegBlackPx = 0;
static uint32_t s_jpegTotalPx = 0;

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

    screenInstance->tft.drawRGBBitmap(x, y, bitmap, w, h);

    // Muestrear cada 8 pixeles para no impactar velocidad
    uint32_t count = (uint32_t)w * h;
    for (uint32_t i = 0; i < count; i += 8) {
        uint16_t px = bitmap[i];
        // RGB565: negro o casi negro  (R<2, G<4, B<2)
        if (px < 0x0842u) s_jpegBlackPx++;
        s_jpegTotalPx++;
    }

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

bool ScreenManager::showJpeg(
    uint8_t* jpgBuffer,
    uint32_t jpgSize,
    int jpg_w,
    int jpg_h
) {

    if (jpgBuffer == nullptr || jpgSize == 0) {

        Serial.println("JPEG invalido");

        return false;
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

    s_jpegBlackPx = 0;
    s_jpegTotalPx = 0;

    TJpgDec.drawJpg(posX, posY, jpgBuffer, jpgSize);

    // =========================================
    // DEBUG
    // =========================================

    // Determinar si el frame es mayormente negro (>80% de muestras)
    bool isBlack = (s_jpegTotalPx > 0) &&
                   (s_jpegBlackPx * 10 >= s_jpegTotalPx * 8);

    Serial.print("[CAM] frame ");
    Serial.print(isBlack ? "NEGRO" : "OK");
    Serial.print("  black=");
    Serial.print(s_jpegBlackPx);
    Serial.print("/");
    Serial.println(s_jpegTotalPx);

    return !isBlack;
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
// DRAW TIMER
// =====================================================

void ScreenManager::drawTimer(int minutes, int seconds, int state, int field, bool flashOn) {
    tft.fillScreen(GC9A01A_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds("TEMPORIZADOR", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 10);
    tft.println("TEMPORIZADOR");

    char buf[8];
    sprintf(buf, "%02d:%02d", minutes, seconds);
    tft.setTextSize(5);

    if (state == 2 && flashOn) {
        tft.setTextColor(GC9A01A_DARKGREY);
    } else if (state == 2) {
        tft.setTextColor(GC9A01A_RED);
    } else if (state == 0) {
        tft.setTextColor(GC9A01A_WHITE);
    } else {
        tft.setTextColor(GC9A01A_GREEN);
    }

    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 65);
    tft.println(buf);

    if (state == 0) {
        int ux = (tft.width() - w) / 2;
        if (field == 0) {
            tft.drawFastHLine(ux, 95, w / 2 - 10, GC9A01A_YELLOW);
        } else {
            tft.drawFastHLine(ux + w / 2 + 10, 95, w / 2 - 10, GC9A01A_YELLOW);
        }
    }

    tft.setTextSize(1);
    if (state == 0) {
        tft.setTextColor(GC9A01A_LIGHTGREY);
        tft.setCursor(10, 220);
        tft.println("[UP/DN] Ajustar  [L/R] Campo  [OK] Iniciar");
    } else if (state == 1) {
        tft.setTextColor(GC9A01A_GREEN);
        tft.setCursor(10, 220);
        tft.println("CORRIENDO  [OK] Pausa");
    } else if (state == 2) {
        tft.setTextColor(GC9A01A_LIGHTGREY);
        tft.setCursor(10, 220);
        tft.println("[OK] Reset");
    }
}

// =====================================================
// DRAW SIMON
// =====================================================

void ScreenManager::drawSimon(int highlight, int score, int state, bool flashOn) {
    // Colors: 0=UP(green), 1=DOWN(red), 2=LEFT(blue), 3=RIGHT(yellow)
    static const uint16_t LIT[4]  = { 0x07E0, 0xF800, 0x001F, 0xFFE0 };
    static const uint16_t DIM[4]  = { 0x01E0, 0x3800, 0x0007, 0x39E0 };

    tft.fillScreen(GC9A01A_BLACK);

    for (int i = 0; i < 4; i++) {
        bool lit;
        if (state == 3) { // LOSE — blink the wrong button
            lit = (highlight == i) && flashOn;
        } else {
            lit = (highlight == i);
        }
        uint16_t color = lit ? LIT[i] : DIM[i];

        switch (i) {
            case 0: tft.fillRoundRect(70,   5, 100, 85, 12, color); break; // UP
            case 1: tft.fillRoundRect(70, 150, 100, 85, 12, color); break; // DOWN
            case 2: tft.fillRoundRect(  5,  70,  85,100, 12, color); break; // LEFT
            case 3: tft.fillRoundRect(150,  70,  85,100, 12, color); break; // RIGHT
        }
    }

    // Center circle
    tft.fillCircle(120, 120, 34, GC9A01A_BLACK);
    tft.drawCircle(120, 120, 34, GC9A01A_DARKGREY);
    tft.drawCircle(120, 120, 33, GC9A01A_DARKGREY);

    char buf[8];
    sprintf(buf, "%d", score);
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(120 - w / 2, 108);
    tft.println(buf);

    // State label
    tft.setTextSize(1);
    const char* label = "";
    uint16_t labelColor = GC9A01A_LIGHTGREY;
    if (state == 0) { label = "Observa...";  labelColor = GC9A01A_CYAN;   }
    if (state == 1) { label = "Tu turno!";   labelColor = GC9A01A_GREEN;  }
    if (state == 2) { label = "Correcto!";   labelColor = GC9A01A_YELLOW; }
    if (state == 3) { label = "Perdiste OK"; labelColor = GC9A01A_RED;    }

    tft.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    tft.setTextColor(labelColor);
    tft.setCursor(120 - w / 2, 228);
    tft.println(label);
}

// =====================================================
// DRAW TETRIS
// =====================================================

void ScreenManager::drawTetris(const uint8_t grid[20][10], const int8_t nextCells[4][2],
                                int nextType, int score, int level, bool gameOver,
                                int heldType, const int8_t heldCells[4][2]) {
    static const uint16_t COLORS[9] = {
        GC9A01A_BLACK,   // 0 vacío
        GC9A01A_CYAN,    // 1 I
        GC9A01A_YELLOW,  // 2 O
        GC9A01A_MAGENTA, // 3 T
        GC9A01A_GREEN,   // 4 S
        GC9A01A_RED,     // 5 Z
        GC9A01A_BLUE,    // 6 J
        GC9A01A_ORANGE,  // 7 L
        0x2965,          // 8 ghost
    };

    const int CELL = 10, SX = 70, SY = 28;

    tft.fillScreen(GC9A01A_BLACK);

    // Puntaje y nivel encima del tablero
    char buf[20];
    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_WHITE);
    sprintf(buf, "SC:%d", score);
    tft.setCursor(SX, 14);
    tft.print(buf);
    sprintf(buf, "LV:%d", level);
    tft.setCursor(SX + 55, 14);
    tft.print(buf);

    // Borde del tablero
    tft.drawRect(SX - 1, SY - 1, 10 * CELL + 2, 20 * CELL + 2, GC9A01A_DARKGREY);

    // Celdas
    for (int r = 0; r < 20; r++) {
        for (int c = 0; c < 10; c++) {
            uint8_t v = grid[r][c];
            if (v > 0 && v <= 8) {
                tft.fillRect(SX + c * CELL + 1, SY + r * CELL + 1,
                             CELL - 2, CELL - 2, COLORS[v]);
            }
        }
    }

    // Panel derecho: siguiente pieza
    const int PX = 178, PY = 88, PREV = 7;
    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(PX, PY - 13);
    tft.print("NEXT");
    tft.drawRect(PX - 1, PY - 1, 4 * PREV + 2, 4 * PREV + 2, GC9A01A_DARKGREY);
    tft.fillRect(PX, PY, 4 * PREV, 4 * PREV, GC9A01A_BLACK);
    uint16_t pc = (nextType >= 0 && nextType <= 6) ? COLORS[nextType + 1] : GC9A01A_WHITE;
    for (int i = 0; i < 4; i++) {
        int nr = nextCells[i][0];
        int nc = nextCells[i][1];
        tft.fillRect(PX + nc * PREV + 1, PY + nr * PREV + 1, PREV - 2, PREV - 2, pc);
    }

    // Panel derecho: pieza reservada (HOLD)
    const int HY = PY + 4 * PREV + 22;
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(PX, HY - 13);
    tft.print("HOLD");
    tft.drawRect(PX - 1, HY - 1, 4 * PREV + 2, 4 * PREV + 2, GC9A01A_DARKGREY);
    tft.fillRect(PX, HY, 4 * PREV, 4 * PREV, GC9A01A_BLACK);
    if (heldType >= 0 && heldType <= 6 && heldCells != nullptr) {
        uint16_t hc = COLORS[heldType + 1];
        for (int i = 0; i < 4; i++) {
            int hr = heldCells[i][0];
            int hcc = heldCells[i][1];
            if (hr >= 0 && hcc >= 0) {
                tft.fillRect(PX + hcc * PREV + 1, HY + hr * PREV + 1, PREV - 2, PREV - 2, hc);
            }
        }
    }

    // Game over
    if (gameOver) {
        int16_t x1, y1;
        uint16_t w, h;
        const char* msg = "GAME OVER";
        tft.setTextSize(2);
        tft.setTextColor(GC9A01A_RED);
        tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
        int ox = (tft.width() - w) / 2;
        int oy = tft.height() / 2 - 12;
        tft.fillRect(ox - 4, oy - 2, w + 8, 28, GC9A01A_BLACK);
        tft.setCursor(ox, oy);
        tft.println(msg);
        tft.setTextSize(1);
        tft.setTextColor(GC9A01A_LIGHTGREY);
        const char* hint = "[A] Reiniciar";
        tft.getTextBounds(hint, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, oy + 16);
        tft.println(hint);
    }
}

// =====================================================
// DRAW 2048
// =====================================================

static uint16_t tile2048Color(int val) {
    switch (val) {
        case 2:    return 0xEF7B;
        case 4:    return 0xEDB0;
        case 8:    return 0xFD20;
        case 16:   return 0xFC00;
        case 32:   return 0xF940;
        case 64:   return 0xF800;
        case 128:  return 0xFFE0;
        case 256:  return 0xFF40;
        case 512:  return 0xFFA0;
        case 1024: return 0x07E0;
        case 2048: return 0x07FF;
        default:   return 0x2104;
    }
}

void ScreenManager::drawGame2048(const int grid[4][4], int score, bool won, bool gameOver) {
    tft.fillScreen(GC9A01A_BLACK);

    // CELL=39, GAP=3 → total 165px. SX=SY=38 → esquinas a ~117px del centro (dentro del círculo r=120)
    const int CELL = 39, GAP = 3, SX = 38, SY = 38;

    char buf[16];

    // Puntaje centrado encima del grid
    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_WHITE);
    sprintf(buf, "2048  SC:%d", score);
    int16_t tx, ty;
    uint16_t tw, th;
    tft.getTextBounds(buf, 0, 0, &tx, &ty, &tw, &th);
    tft.setCursor((tft.width() - tw) / 2, 24);
    tft.print(buf);

    // Grid
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = grid[r][c];
            int x   = SX + c * (CELL + GAP);
            int y   = SY + r * (CELL + GAP);

            tft.fillRoundRect(x, y, CELL, CELL, 4, tile2048Color(val));

            if (val > 0) {
                sprintf(buf, "%d", val);
                // tamaño de texto según dígitos
                uint8_t ts = (val < 100) ? 2 : 1;
                tft.setTextSize(ts);
                uint16_t tc = (val <= 4) ? (uint16_t)0x2104 : (uint16_t)GC9A01A_WHITE;
                tft.setTextColor(tc);
                int16_t x1, y1;
                uint16_t w, h;
                tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
                tft.setCursor(x + (CELL - w) / 2, y + (CELL - h) / 2);
                tft.print(buf);
            }
        }
    }

    if (won || gameOver) {
        const char* msg = won ? "YOU WIN!" : "GAME OVER";
        tft.setTextSize(2);
        tft.setTextColor(won ? GC9A01A_YELLOW : GC9A01A_RED);
        int16_t x1, y1;
        uint16_t w, h;
        tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
        int oy = tft.height() / 2 - 12;
        tft.fillRect(0, oy - 4, tft.width(), 36, GC9A01A_BLACK);
        tft.setCursor((tft.width() - w) / 2, oy);
        tft.println(msg);
        tft.setTextSize(1);
        tft.setTextColor(GC9A01A_LIGHTGREY);
        const char* hint = "[OK] Reiniciar";
        tft.getTextBounds(hint, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, oy + 18);
        tft.println(hint);
    }
}

// =====================================================
// DRAW MORSE
// =====================================================

void ScreenManager::drawMorse(const char* pattern, const char* output, char lastDecoded) {
    tft.fillScreen(GC9A01A_BLACK);

    // Título
    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_CYAN);
    tft.getTextBounds("MORSE", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 8);
    tft.println("MORSE");
    tft.drawFastHLine(10, 30, 220, GC9A01A_DARKGREY);

    // Patrón actual
    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(12, 40);
    tft.print("Patron:");

    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    tft.setCursor(12, 52);
    tft.print(pattern[0] ? pattern : "_");

    // Carácter decodificado (grande, a la derecha)
    if (lastDecoded != ' ' && lastDecoded != '\0') {
        char ch[2] = { lastDecoded, '\0' };
        tft.setTextSize(4);
        tft.setTextColor(GC9A01A_GREEN);
        tft.getTextBounds(ch, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor(200 - w, 42);
        tft.print(ch);
    }

    tft.drawFastHLine(10, 80, 220, GC9A01A_DARKGREY);

    // Texto decodificado
    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(12, 90);
    tft.print("Texto:");

    tft.setTextColor(GC9A01A_YELLOW);
    tft.setTextSize(1);
    // Mostrar los últimos chars del output (wrap automático)
    int outLen = strlen(output);
    const int maxShow = 84; // ~3 líneas × 28 chars
    const char* showFrom = outLen > maxShow ? output + outLen - maxShow : output;
    tft.setCursor(12, 102);
    tft.setTextWrap(true);
    tft.print(showFrom);
    tft.setTextWrap(false);

    // Instrucciones
    tft.drawFastHLine(10, 188, 220, GC9A01A_DARKGREY);
    tft.setTextSize(1);
    tft.setTextColor(0x4208);
    tft.setCursor(12, 196);
    tft.print("[A]=.  [B]=-  [OK]=Enter");
    tft.setCursor(12, 208);
    tft.print("[DN]=Borrar  [UP]=LimpiarP");
}

// =====================================================
// DRAW MUSIC PLAYER
// =====================================================

void ScreenManager::drawMusicPlayer(const char* name, int noteIdx, int total,
                                    bool playing, bool paused) {
    tft.fillScreen(GC9A01A_BLACK);

    // Título de la canción
    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    tft.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 18);
    tft.println(name);

    tft.drawFastHLine(20, 38, 200, GC9A01A_DARKGREY);

    // Animación visual: círculos concéntricos que pulsan con cada nota
    int cx = 120, cy = 118;
    int pulse = noteIdx % 6;
    static const uint16_t ringColors[6] = {
        GC9A01A_CYAN, GC9A01A_BLUE, GC9A01A_MAGENTA,
        GC9A01A_GREEN, GC9A01A_YELLOW, GC9A01A_RED
    };

    if (!paused) {
        for (int i = 0; i < 4; i++) {
            int r = 18 + i * 16 + pulse * 3;
            uint16_t c = ringColors[(pulse + i) % 6];
            tft.drawCircle(cx, cy, r, c);
            tft.drawCircle(cx, cy, r + 1, c);
        }
    } else {
        // Paused: círculos fijos grises
        for (int i = 0; i < 4; i++) {
            tft.drawCircle(cx, cy, 18 + i * 16, GC9A01A_DARKGREY);
        }
    }

    // Icono estado (▶ / ▐▐)
    tft.setTextSize(1);
    const char* statusStr = paused ? "II PAUSADO" : (playing ? ">  TOCANDO" : "   PARADO");
    uint16_t statusColor  = paused ? GC9A01A_YELLOW : (playing ? GC9A01A_GREEN : GC9A01A_DARKGREY);
    tft.setTextColor(statusColor);
    tft.getTextBounds(statusStr, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 172);
    tft.println(statusStr);

    // Barra de progreso
    if (total > 0) {
        const int bx = 30, by = 186, bw = 180, bh = 6;
        tft.drawRect(bx, by, bw, bh, GC9A01A_DARKGREY);
        int filled = (noteIdx * bw) / total;
        if (filled > 0)
            tft.fillRect(bx, by, filled, bh, playing ? GC9A01A_CYAN : GC9A01A_DARKGREY);

        // Porcentaje
        char pct[8];
        int percent = (noteIdx * 100) / total;
        sprintf(pct, "%d%%", percent);
        tft.setTextColor(GC9A01A_LIGHTGREY);
        tft.getTextBounds(pct, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, 198);
        tft.println(pct);
    }

    // Instrucciones
    tft.setTextColor(0x4208);
    tft.setCursor(28, 216);
    tft.print("[OK] Play/Pausa  [MNU] Volver");
}

// =====================================================
// DRAW SYNTH
// =====================================================

// Notas de la octava (0=C … 11=B). Las negras: índices 1,3,6,8,10
static const bool SYNTH_IS_BLACK[12] = {
    false,true,false,true,false,false,true,false,true,false,true,false
};
static const char* SYNTH_NOTE_NAMES[12] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};
// Colores de las 12 teclas cuando están activas
static const uint16_t SYNTH_KEY_COLORS[12] = {
    GC9A01A_RED, 0xF800,
    GC9A01A_YELLOW, 0xFFE0,
    GC9A01A_GREEN,
    GC9A01A_CYAN, 0x07FF,
    GC9A01A_BLUE, 0x001F,
    GC9A01A_MAGENTA, 0xF81F,
    0xF800
};
// Posiciones del anillo de 12 notas (offsets desde el centro, R=56,
// empezando arriba y girando en sentido horario en pasos de 30°)
static const int8_t SYNTH_RING_DX[12] = {  0, 28, 48, 56, 48, 28,  0,-28,-48,-56,-48,-28};
static const int8_t SYNTH_RING_DY[12] = {-56,-48,-28,  0, 28, 48, 56, 48, 28,  0,-28,-48};

void ScreenManager::drawSynth(const SynthView& v) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t x1, y1;
    uint16_t w, h;

    // --- Título ---
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_CYAN);
    tft.getTextBounds("SYNTH", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 4);
    tft.print("SYNTH");

    // --- Etiqueta de modo ---
    tft.setTextSize(1);
    const char* modeStr = (v.mode == 0) ? "TOCAR" : (v.mode == 1) ? "TEMPO" : "SECUENCIA";
    uint16_t modeCol = (v.mode == 0) ? GC9A01A_GREEN : (v.mode == 1) ? GC9A01A_CYAN : GC9A01A_YELLOW;
    tft.setTextColor(modeCol);
    tft.getTextBounds(modeStr, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 26);
    tft.print(modeStr);

    // --- Anillo circular de 12 notas ---
    const int cx = 120, cy = 92;
    for (int i = 0; i < 12; i++) {
        int px = cx + SYNTH_RING_DX[i];
        int py = cy + SYNTH_RING_DY[i];
        bool sel = (i == v.noteIdx);
        if (sel) {
            tft.fillCircle(px, py, 11, SYNTH_KEY_COLORS[i]);
            tft.drawCircle(px, py, 12, GC9A01A_WHITE);
            tft.drawCircle(px, py, 13, GC9A01A_WHITE);
        } else {
            tft.fillCircle(px, py, 6, SYNTH_IS_BLACK[i] ? 0x4208 : 0x8410);
        }
    }

    // --- Nombre de nota grande en el centro ---
    char noteBuf[6];
    snprintf(noteBuf, sizeof(noteBuf), "%s%d", SYNTH_NOTE_NAMES[v.noteIdx], v.octave);
    tft.setTextSize(3);
    tft.setTextColor(v.playing ? SYNTH_KEY_COLORS[v.noteIdx] : GC9A01A_WHITE);
    tft.getTextBounds(noteBuf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(cx - w / 2, cy - h / 2);
    tft.print(noteBuf);

    // --- Secuencia (fila de pasos) ---
    const int sy = 156, bh = 18;
    if (v.seqLen > 0) {
        int n = v.seqLen, gap = 3, bw = 20;
        int total = n * bw + (n - 1) * gap;
        if (total > 196) { bw = (196 - (n - 1) * gap) / n; total = n * bw + (n - 1) * gap; }
        int sx = (240 - total) / 2;
        for (int i = 0; i < n; i++) {
            int bx = sx + i * (bw + gap);
            uint16_t bc = SYNTH_KEY_COLORS[v.seqNotes[i]];
            bool isPlay = (v.playStep == i);
            bool isCur  = (v.mode == 2 && v.seqCursor == i);
            if (isPlay) {
                tft.fillRect(bx, sy, bw, bh, bc);
            } else {
                tft.fillRect(bx, sy, bw, bh, 0x2104);
                tft.drawRect(bx, sy, bw, bh, bc);
            }
            if (isCur) tft.drawRect(bx - 1, sy - 1, bw + 2, bh + 2, GC9A01A_WHITE);

            tft.setTextSize(1);
            tft.setTextColor(isPlay ? GC9A01A_BLACK : GC9A01A_WHITE);
            const char* nm = SYNTH_NOTE_NAMES[v.seqNotes[i]];
            int tw = (nm[1] ? 11 : 5);
            tft.setCursor(bx + (bw - tw) / 2, sy + 5);
            tft.print(nm);

            // Badge de duracion (tiempos) cuando es mayor a 1
            if (v.seqDurs[i] > 1) {
                char db[4];
                snprintf(db, sizeof(db), "%d", v.seqDurs[i]);
                tft.setTextColor(isPlay ? GC9A01A_BLACK : GC9A01A_CYAN);
                tft.setCursor(bx + bw - 6, sy + 1);
                tft.print(db);
            }
        }
    } else {
        tft.setTextSize(1);
        tft.setTextColor(0x4208);
        const char* e = "(secuencia vacia)";
        tft.getTextBounds(e, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((240 - w) / 2, sy + 5);
        tft.print(e);
    }

    // --- Estado del bucle + tempo (resaltado en modo TEMPO) ---
    tft.setTextSize(1);
    uint16_t tempoCol = (v.mode == 1) ? GC9A01A_CYAN
                                      : (v.seqPlaying ? GC9A01A_GREEN : GC9A01A_DARKGREY);
    tft.setTextColor(tempoCol);
    char tb[20];
    snprintf(tb, sizeof(tb), "%s  %d BPM", v.seqPlaying ? "LOOP" : "STOP", v.tempoBpm);
    tft.getTextBounds(tb, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 184);
    tft.print(tb);

    // --- Instrucciones segun modo ---
    tft.setTextColor(0x4208);
    if (v.mode == 0) {
        tft.setCursor(18, 200); tft.print("<> Nota  ^v Oct  OK Toca");
        tft.setCursor(36, 211); tft.print("B Agregar    A Modo");
    } else if (v.mode == 1) {
        tft.setCursor(22, 200); tft.print("^v Tempo   OK Loop");
        tft.setCursor(70, 211); tft.print("A Modo");
    } else {
        tft.setCursor(14, 200); tft.print("<> Paso  ^v Tiempos  OK Loop");
        tft.setCursor(36, 211); tft.print("B Borrar     A Modo");
    }
}

// =====================================================
// DRAW LINTERNA
// =====================================================

// Colores TFT para cada índice (debe coincidir con LinternaManager)
static const uint16_t LINTERNA_TFT_COLORS[8] = {
    GC9A01A_BLUE,
    GC9A01A_WHITE,
    GC9A01A_RED,
    GC9A01A_GREEN,
    GC9A01A_CYAN,
    GC9A01A_MAGENTA,
    GC9A01A_YELLOW,
    GC9A01A_ORANGE,
};
static const char* LINTERNA_TFT_NAMES[8] = {
    "Azul", "Blanco", "Rojo", "Verde",
    "Cian", "Magenta", "Amarillo", "Naranja",
};

void ScreenManager::drawLinterna(uint8_t colorIndex, uint8_t brightness, bool on) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t x1, y1;
    uint16_t w, h;

    // Título
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    tft.getTextBounds("LINTERNA", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 10);
    tft.print("LINTERNA");
    tft.drawFastHLine(20, 30, 200, GC9A01A_DARKGREY);

    // Círculo central con el color del LED
    int cx = tft.width() / 2;
    int cy = 110;
    int cr = 52;
    uint16_t col = on ? LINTERNA_TFT_COLORS[colorIndex] : (uint16_t)GC9A01A_DARKGREY;
    tft.fillCircle(cx, cy, cr, col);
    // Anillo exterior
    tft.drawCircle(cx, cy, cr + 2, on ? col : (uint16_t)0x4208);
    tft.drawCircle(cx, cy, cr + 3, on ? col : (uint16_t)0x4208);

    // Icono ON/OFF dentro del círculo
    tft.setTextSize(2);
    uint16_t iconColor = on ? GC9A01A_BLACK : GC9A01A_DARKGREY;
    const char* iconStr = on ? "ON" : "OFF";
    tft.setTextColor(iconColor);
    tft.getTextBounds(iconStr, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(cx - w / 2, cy - h / 2);
    tft.print(iconStr);

    // Nombre del color
    tft.setTextSize(1);
    tft.setTextColor(on ? LINTERNA_TFT_COLORS[colorIndex] : (uint16_t)GC9A01A_DARKGREY);
    const char* colName = LINTERNA_TFT_NAMES[colorIndex];
    tft.getTextBounds(colName, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(cx - w / 2, 174);
    tft.print(colName);

    // Barra de brillo
    const int bx = 30, by = 186, bw = 180, bh = 7;
    tft.drawRect(bx, by, bw, bh, GC9A01A_DARKGREY);
    int filled = ((int)brightness * bw) / 255;
    if (filled > 0 && on)
        tft.fillRect(bx, by, filled, bh, LINTERNA_TFT_COLORS[colorIndex]);

    // Porcentaje de brillo
    char pct[8];
    sprintf(pct, "%d%%", (brightness * 100) / 255);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.getTextBounds(pct, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(cx - w / 2, 198);
    tft.print(pct);

    // Instrucciones
    tft.setTextColor(0x4208);
    tft.setCursor(14, 215);
    tft.print("<> Color  ^v Brillo  [OK] ON");
}

// =====================================================
// DRAW SIRENA
// =====================================================

void ScreenManager::drawSirena(const char* pattern, bool on, uint8_t brightness) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t x1, y1;
    uint16_t w, h;

    // Título
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_RED);
    tft.getTextBounds("SIRENA", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 10);
    tft.print("SIRENA");
    tft.drawFastHLine(20, 30, 200, GC9A01A_DARKGREY);

    // Círculo central tipo baliza
    int cx = 120, cy = 104, cr = 48;
    uint16_t col = on ? GC9A01A_RED : (uint16_t)GC9A01A_DARKGREY;
    tft.fillCircle(cx, cy, cr, col);
    tft.drawCircle(cx, cy, cr + 2, on ? (uint16_t)GC9A01A_BLUE : (uint16_t)0x4208);
    tft.drawCircle(cx, cy, cr + 3, on ? (uint16_t)GC9A01A_BLUE : (uint16_t)0x4208);

    // Estado ON/OFF dentro del círculo
    tft.setTextSize(2);
    tft.setTextColor(on ? GC9A01A_WHITE : GC9A01A_DARKGREY);
    const char* st = on ? "ON" : "OFF";
    tft.getTextBounds(st, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(cx - w / 2, cy - h / 2);
    tft.print(st);

    // Patrón
    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_CYAN);
    char pbuf[24];
    snprintf(pbuf, sizeof(pbuf), "Patron: %s", pattern);
    tft.getTextBounds(pbuf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 168);
    tft.print(pbuf);

    // Barra de brillo del LED
    const int bx = 30, by = 184, bw = 180, bh = 7;
    tft.drawRect(bx, by, bw, bh, GC9A01A_DARKGREY);
    int filled = ((int)brightness * bw) / 255;
    if (filled > 0) tft.fillRect(bx, by, filled, bh, GC9A01A_RED);

    char pct[8];
    sprintf(pct, "%d%%", (brightness * 100) / 255);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.getTextBounds(pct, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(cx - w / 2, 196);
    tft.print(pct);

    // Instrucciones
    tft.setTextColor(0x4208);
    tft.setCursor(10, 215);
    tft.print("OK On/Off  <> Patron  ^v Brillo");
}

// =====================================================
// DRAW CAM CAR
// =====================================================

void ScreenManager::drawCamCarStatus(bool active, const char* cmdName, bool linked) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t x1, y1;
    uint16_t w, h;

    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_CYAN);
    tft.getTextBounds("CAM CAR", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 10);
    tft.print("CAM CAR");
    tft.drawFastHLine(20, 34, 200, GC9A01A_DARKGREY);

    // Estado del enlace ESP-NOW
    tft.setTextSize(1);
    tft.setTextColor(active ? (linked ? GC9A01A_GREEN : GC9A01A_ORANGE) : GC9A01A_DARKGREY);
    const char* st = !active ? "ESP-NOW apagado"
                    : linked ? "Enlazado"
                             : "Esperando video...";
    tft.getTextBounds(st, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 48);
    tft.print(st);

    // Icono de direccion central
    int cx = 120, cy = 120, r = 50;
    tft.drawCircle(cx, cy, r, active ? GC9A01A_CYAN : (uint16_t)0x4208);
    tft.drawCircle(cx, cy, r - 1, active ? GC9A01A_CYAN : (uint16_t)0x4208);

    tft.setTextSize(3);
    tft.setTextColor(active ? GC9A01A_WHITE : GC9A01A_DARKGREY);
    tft.getTextBounds(cmdName, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(cx - w / 2, cy - h / 2);
    tft.print(cmdName);

    tft.setTextSize(1);
    tft.setTextColor(0x4208);
    tft.setCursor(8, 200);
    tft.print("^v<> Mover  OK Stop  A ESP-NOW");
    tft.setCursor(40, 211);
    tft.print("MENU Salir");
}

void ScreenManager::drawCamCarOverlay(const char* cmdName, bool linked, uint32_t fps) {
    // HUD compacto dibujado encima del frame de video ya pintado.
    tft.fillRect(0, 0, 240, 14, GC9A01A_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(linked ? GC9A01A_GREEN : GC9A01A_ORANGE);
    tft.setCursor(4, 3);
    tft.print(linked ? "LIVE" : "...");

    tft.setTextColor(GC9A01A_CYAN);
    tft.setCursor(60, 3);
    tft.print(cmdName);

    char fb[12];
    snprintf(fb, sizeof(fb), "%lu fps", (unsigned long)fps);
    tft.setTextColor(GC9A01A_WHITE);
    int16_t x1, y1; uint16_t w, h;
    tft.getTextBounds(fb, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(236 - w, 3);
    tft.print(fb);
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

// =====================================================
// DRAW BT KEYBOARD
// =====================================================

// Dibuja una tecla con su etiqueta centrada. Resaltada = verde.
static void btKeycap(Adafruit_GC9A01A& tft, int x, int y, int w, int h,
                     const char* label, bool pressed) {
    uint16_t fill   = pressed ? GC9A01A_GREEN : 0x2124;      // gris oscuro
    uint16_t border = pressed ? GC9A01A_WHITE : 0x4A49;
    uint16_t txt    = pressed ? GC9A01A_BLACK : GC9A01A_WHITE;

    tft.fillRoundRect(x, y, w, h, 4, fill);
    tft.drawRoundRect(x, y, w, h, 4, border);

    tft.setTextSize(1);
    tft.setTextColor(txt);
    int16_t bx, by; uint16_t bw, bh;
    tft.getTextBounds(label, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor(x + (w - bw) / 2, y + (h - bh) / 2);
    tft.print(label);
}

void ScreenManager::drawBtKeyboard(bool active, bool connected,
                                    const char* deviceName, uint8_t pressedMask) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t bx, by; uint16_t bw, bh;

    // Titulo
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_CYAN);
    const char* title = "TECLADO";
    tft.getTextBounds(title, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 16);
    tft.print(title);

    // Estado de conexion
    const char* status;
    uint16_t statusColor;
    if (!active)        { status = "BT APAGADO";   statusColor = GC9A01A_DARKGREY; }
    else if (connected) { status = "CONECTADO";    statusColor = GC9A01A_GREEN;    }
    else                { status = "EMPAREJAR...";  statusColor = GC9A01A_YELLOW;   }

    tft.setTextSize(1);
    tft.setTextColor(statusColor);
    tft.getTextBounds(status, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 40);
    tft.print(status);

    // Nombre visible al emparejar (solo si aun no hay host)
    if (active && !connected && deviceName) {
        tft.setTextColor(0x7BEF);
        tft.getTextBounds(deviceName, 0, 0, &bx, &by, &bw, &bh);
        tft.setCursor((240 - bw) / 2, 53);
        tft.print(deviceName);
    }

    // Cruz WASD: refleja el d-pad fisico (arriba=W, izq=A, abajo=S, der=D)
    const int KW = 32, KH = 26;
    btKeycap(tft, 104,  74, KW, KH, "W", pressedMask & 0x01);
    btKeycap(tft,  66, 104, KW, KH, "A", pressedMask & 0x02);
    btKeycap(tft, 104, 134, KW, KH, "S", pressedMask & 0x04);
    btKeycap(tft, 142, 104, KW, KH, "D", pressedMask & 0x08);

    // Fila inferior: teclas de accion
    btKeycap(tft,  42, 176, 44, 24, "ESC", pressedMask & 0x10);
    btKeycap(tft,  94, 176, 48, 24, "ENTER", pressedMask & 0x20);
    btKeycap(tft, 150, 176, 48, 24, "SPACE", pressedMask & 0x40);

    // Leyenda de botones fisicos
    tft.setTextSize(1);
    tft.setTextColor(0x39C7);
    const char* hint = "[B]esc [OK]enter [A]space";
    tft.getTextBounds(hint, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 208);
    tft.print(hint);
}

// =====================================================
// DRAW JOYSTICK
// =====================================================

void ScreenManager::drawJoystick(int rawX, int rawY, int centerX, int centerY,
                                  float x, float y, float magnitude, float angle,
                                  bool up, bool down, bool left, bool right) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t bx, by; uint16_t bw, bh;

    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_CYAN);
    const char* title = "JOYSTICK";
    tft.getTextBounds(title, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 10);
    tft.print(title);

    // --- Area circular con la posicion del stick ---
    const int CX = 120, CY = 104, R = 52;
    tft.drawCircle(CX, CY, R, 0x4A49);
    tft.drawFastHLine(CX - R, CY, R * 2, 0x2124);
    tft.drawFastVLine(CX, CY - R, R * 2, 0x2124);

    // Zona muerta (18% del radio), para ver cuando el stick cuenta como suelto
    tft.drawCircle(CX, CY, (int)(R * 0.18f), 0x39C7);

    // El eje Y de pantalla crece hacia abajo: se invierte para que arriba
    // en el stick se dibuje arriba.
    int px = CX + (int)(x * R);
    int py = CY - (int)(y * R);
    tft.drawLine(CX, CY, px, py, 0x632C);
    tft.fillCircle(px, py, 6, magnitude > 0.0f ? GC9A01A_GREEN : GC9A01A_DARKGREY);

    // --- Direcciones discretas activas ---
    uint16_t on = GC9A01A_GREEN, off = 0x2124;
    tft.fillTriangle(CX - 7, CY - R - 10, CX + 7, CY - R - 10, CX, CY - R - 20, up    ? on : off);
    tft.fillTriangle(CX - 7, CY + R + 10, CX + 7, CY + R + 10, CX, CY + R + 20, down  ? on : off);
    tft.fillTriangle(CX - R - 10, CY - 7, CX - R - 10, CY + 7, CX - R - 20, CY, left  ? on : off);
    tft.fillTriangle(CX + R + 10, CY - 7, CX + R + 10, CY + 7, CX + R + 20, CY, right ? on : off);

    // --- Numeros ---
    char buf[40];
    tft.setTextColor(GC9A01A_LIGHTGREY);

    snprintf(buf, sizeof(buf), "raw %4d,%4d  c %4d,%4d", rawX, rawY, centerX, centerY);
    tft.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 178);
    tft.print(buf);

    snprintf(buf, sizeof(buf), "x %+.2f  y %+.2f", x, y);
    tft.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 191);
    tft.print(buf);

    snprintf(buf, sizeof(buf), "mag %.2f  ang %3.0f", magnitude, angle);
    tft.getTextBounds(buf, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 204);
    tft.print(buf);

    tft.setTextColor(0x39C7);
    const char* hint = "[OK] recalibrar centro";
    tft.getTextBounds(hint, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 218);
    tft.print(hint);
}

// =====================================================
// DRAW LED STRIP
// =====================================================

// Fila de parametro: nombre, barra proporcional y valor numerico.
static void ledParamRow(Adafruit_GC9A01A& tft, int y, const char* name,
                        int value, int maxValue, uint16_t barColor,
                        bool selected, const char* textValue) {
    const int BAR_X = 92, BAR_W = 76, BAR_H = 7;

    if (selected) {
        tft.fillRoundRect(14, y - 3, 212, 17, 3, 0x18E3);
        tft.drawRoundRect(14, y - 3, 212, 17, 3, GC9A01A_WHITE);
    }

    tft.setTextSize(1);
    tft.setTextColor(selected ? GC9A01A_WHITE : 0x8410);
    tft.setCursor(22, y + 1);
    tft.print(name);

    if (textValue) {
        // Fila de efecto: se muestra el nombre en vez de una barra
        tft.setTextColor(selected ? GC9A01A_YELLOW : GC9A01A_LIGHTGREY);
        tft.setCursor(BAR_X, y + 1);
        tft.print(textValue);
        return;
    }

    tft.drawRect(BAR_X, y, BAR_W, BAR_H, 0x4A49);
    int filled = (maxValue > 0) ? (value * (BAR_W - 2)) / maxValue : 0;
    if (filled > 0) tft.fillRect(BAR_X + 1, y + 1, filled, BAR_H - 2, barColor);

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", value);
    tft.setTextColor(selected ? GC9A01A_WHITE : GC9A01A_LIGHTGREY);
    tft.setCursor(BAR_X + BAR_W + 8, y + 1);
    tft.print(buf);
}

void ScreenManager::drawLedStrip(int param, uint8_t effect, const char* effectName,
                                  uint8_t brightness, uint8_t r, uint8_t g, uint8_t b,
                                  uint8_t bps, bool active, bool lastOk, bool broadcast,
                                  uint32_t sent, uint32_t failed) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t bx, by; uint16_t bw, bh;

    // Titulo
    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_CYAN);
    const char* title = "TIRA LED";
    tft.getTextBounds(title, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 12);
    tft.print(title);

    // Muestra del color activo, con el brillo aplicado para que se parezca
    // a lo que realmente se vera en la tira
    uint16_t swatch = tft.color565((r * brightness) / 255,
                                   (g * brightness) / 255,
                                   (b * brightness) / 255);
    tft.fillRoundRect(85, 26, 70, 20, 4, swatch);
    tft.drawRoundRect(85, 26, 70, 20, 4, 0x632C);

    // Filas de parametros
    const int Y0 = 56, STEP = 18;
    ledParamRow(tft, Y0 + 0 * STEP, "EFECTO", effect, LED_FX_COUNT - 1,
                GC9A01A_MAGENTA, param == 0, effectName);
    ledParamRow(tft, Y0 + 1 * STEP, "BRILLO", brightness, 255,
                GC9A01A_WHITE, param == 1, nullptr);
    ledParamRow(tft, Y0 + 2 * STEP, "ROJO", r, 255,
                GC9A01A_RED, param == 2, nullptr);
    ledParamRow(tft, Y0 + 3 * STEP, "VERDE", g, 255,
                GC9A01A_GREEN, param == 3, nullptr);
    ledParamRow(tft, Y0 + 4 * STEP, "AZUL", b, 255,
                GC9A01A_BLUE, param == 4, nullptr);
    ledParamRow(tft, Y0 + 5 * STEP, "VELOC", bps, 60,
                GC9A01A_ORANGE, param == 5, nullptr);

    // Estado del enlace. El ACK es de radio: confirma que alguien lo recibio,
    // no que la tira lo haya aplicado.
    tft.setTextSize(1);
    char status[36];
    if (!active) {
        tft.setTextColor(GC9A01A_DARKGREY);
        snprintf(status, sizeof(status), "RADIO APAGADA");
    } else if (sent == 0 && failed == 0) {
        tft.setTextColor(GC9A01A_YELLOW);
        snprintf(status, sizeof(status), "%s  sin enviar",
                 broadcast ? "BCAST" : "DIRECTO");
    } else {
        tft.setTextColor(lastOk ? GC9A01A_GREEN : GC9A01A_RED);
        snprintf(status, sizeof(status), "%s  ok:%u fallo:%u",
                 broadcast ? "BCAST" : "DIRECTO",
                 (unsigned)sent, (unsigned)failed);
    }
    tft.getTextBounds(status, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 172);
    tft.print(status);

    // Ayuda
    tft.setTextColor(0x39C7);
    const char* h1 = "<> param   ^v valor";
    tft.getTextBounds(h1, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 192);
    tft.print(h1);

    const char* h2 = "[A]efecto [B]negro [OK]envia";
    tft.getTextBounds(h2, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 206);
    tft.print(h2);
}

// =====================================================
// DRAW MUSIC CONTROL
// =====================================================

void ScreenManager::drawMusicControl(bool active, bool connected,
                                      const char* deviceName,
                                      bool playing, const char* lastAction) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t bx, by; uint16_t bw, bh;

    // Titulo
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_CYAN);
    const char* title = "MUSICA";
    tft.getTextBounds(title, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 14);
    tft.print(title);

    // Estado de conexion
    const char* status;
    uint16_t statusColor;
    if (!active)        { status = "BT APAGADO";  statusColor = GC9A01A_DARKGREY; }
    else if (connected) { status = "CONECTADO";   statusColor = GC9A01A_GREEN;    }
    else                { status = "EMPAREJAR..."; statusColor = GC9A01A_YELLOW;   }

    tft.setTextSize(1);
    tft.setTextColor(statusColor);
    tft.getTextBounds(status, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 36);
    tft.print(status);

    if (active && !connected && deviceName) {
        tft.setTextColor(0x7BEF);
        tft.getTextBounds(deviceName, 0, 0, &bx, &by, &bw, &bh);
        tft.setCursor((240 - bw) / 2, 48);
        tft.print(deviceName);
    }

    // --- Transporte ---
    const int CX = 120, CY = 112;
    uint16_t glyph = connected ? GC9A01A_GREEN : GC9A01A_DARKGREY;

    tft.drawCircle(CX, CY, 34, 0x4A49);
    if (playing) {
        // Suena: se muestran las barras de pausa (lo que hara [OK])
        tft.fillRect(CX - 13, CY - 16, 9, 32, glyph);
        tft.fillRect(CX +  4, CY - 16, 9, 32, glyph);
    } else {
        tft.fillTriangle(CX - 10, CY - 17, CX - 10, CY + 17, CX + 17, CY, glyph);
    }

    // Anterior / siguiente a los lados
    uint16_t side = connected ? GC9A01A_WHITE : GC9A01A_DARKGREY;
    tft.fillTriangle(58, CY - 12, 58, CY + 12, 42, CY, side);
    tft.fillTriangle(74, CY - 12, 74, CY + 12, 58, CY, side);
    tft.fillTriangle(182, CY - 12, 182, CY + 12, 198, CY, side);
    tft.fillTriangle(166, CY - 12, 166, CY + 12, 182, CY, side);

    // Volumen arriba / abajo
    tft.fillTriangle(CX - 9, 70, CX + 9, 70, CX, 60, side);
    tft.fillTriangle(CX - 9, 154, CX + 9, 154, CX, 164, side);

    // Etiqueta del ultimo comando enviado
    if (lastAction && lastAction[0]) {
        tft.setTextSize(2);
        tft.setTextColor(GC9A01A_YELLOW);
        tft.getTextBounds(lastAction, 0, 0, &bx, &by, &bw, &bh);
        tft.setCursor((240 - bw) / 2, 182);
        tft.print(lastAction);
    }

    // Ayuda de botones
    tft.setTextSize(1);
    tft.setTextColor(0x39C7);
    const char* hint = "^v vol  <> pista  [OK] play";
    tft.getTextBounds(hint, 0, 0, &bx, &by, &bw, &bh);
    tft.setCursor((240 - bw) / 2, 210);
    tft.print(hint);
}

// =====================================================
// DRAW PONG
// =====================================================

void ScreenManager::drawPong(int ballX, int ballY, int playerY, int aiY,
                              int pScore, int aScore, bool gameOver, bool playerWon) {
    const int OFX = 20, OFY = 42;
    const int W = 200, H = 150;
    const int PAD_H = 24, PAD_W = 4, BALL = 4;

    tft.fillScreen(GC9A01A_BLACK);

    // Score
    tft.setTextColor(GC9A01A_WHITE); tft.setTextSize(2);
    char buf[8];
    sprintf(buf, "%d", aScore);
    tft.setCursor(OFX + 60, OFY - 28); tft.print(buf);
    sprintf(buf, "%d", pScore);
    tft.setCursor(OFX + 120, OFY - 28); tft.print(buf);
    tft.setCursor(OFX + 96, OFY - 28); tft.print(":");

    // Border
    tft.drawRect(OFX, OFY, W, H, GC9A01A_DARKGREY);
    tft.drawFastVLine(OFX + W / 2, OFY, H, 0x39C7); // center dashed line (grey)

    // AI paddle (left)
    tft.fillRect(OFX + 6, OFY + aiY - PAD_H / 2, PAD_W, PAD_H, GC9A01A_CYAN);
    // Player paddle (right)
    tft.fillRect(OFX + W - 6 - PAD_W, OFY + playerY - PAD_H / 2, PAD_W, PAD_H, GC9A01A_GREEN);
    // Ball
    tft.fillRect(OFX + ballX, OFY + ballY, BALL, BALL, GC9A01A_WHITE);

    if (gameOver) {
        tft.setTextSize(2);
        tft.setTextColor(playerWon ? GC9A01A_GREEN : GC9A01A_RED);
        const char* msg = playerWon ? "GANASTE!" : "PERDISTE";
        int16_t x1, y1; uint16_t w, h;
        tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor(120 - w / 2, 115);
        tft.println(msg);
    } else {
        tft.setTextSize(1); tft.setTextColor(0x39C7);
        tft.setCursor(OFX + 2, OFY + H + 3); tft.print("^v mover  [MENU] salir");
    }
}

// =====================================================
// DRAW BREAKOUT
// =====================================================

void ScreenManager::drawBreakout(const bool bricks[][7], int rows, int cols,
                                  int brickW, int brickH, int padX,
                                  int ballX, int ballY,
                                  int score, int lives, bool launched,
                                  bool gameOver, bool won) {
    const int OFX = 29, OFY = 22;
    const int BALL_R = 3;
    const int PAD_W = 32, PAD_Y = 168, PAD_H = 4;
    const int BRICK_OFY = 14;

    static const uint16_t BRICK_COLORS[] = {
        GC9A01A_RED, GC9A01A_ORANGE, GC9A01A_YELLOW, GC9A01A_GREEN, GC9A01A_CYAN
    };

    tft.fillScreen(GC9A01A_BLACK);

    // HUD
    tft.setTextColor(GC9A01A_WHITE); tft.setTextSize(1);
    tft.setCursor(OFX, OFY - 12);
    char buf[20]; sprintf(buf, "Puntos:%d", score);
    tft.print(buf);
    tft.setCursor(OFX + 110, OFY - 12);
    sprintf(buf, "Vidas:%d", lives);
    tft.print(buf);

    // Bricks
    for (int r = 0; r < rows; r++) {
        uint16_t c16 = BRICK_COLORS[r % 5];
        for (int c = 0; c < cols; c++) {
            if (!bricks[r][c]) continue;
            int bx = OFX + c * brickW;
            int by = OFY + BRICK_OFY + r * brickH;
            tft.fillRect(bx + 1, by + 1, brickW - 2, brickH - 2, c16);
        }
    }

    // Paddle
    tft.fillRect(OFX + padX, OFY + PAD_Y, PAD_W, PAD_H, GC9A01A_WHITE);

    // Ball
    tft.fillCircle(OFX + ballX, OFY + ballY, BALL_R, GC9A01A_WHITE);

    if (!launched) {
        tft.setTextColor(GC9A01A_YELLOW); tft.setTextSize(1);
        tft.setCursor(55, 215); tft.print("[OK] lanzar");
    }
    if (gameOver) {
        tft.setTextColor(GC9A01A_RED); tft.setTextSize(2);
        tft.setCursor(60, 110); tft.print("GAME OVER");
    }
    if (won) {
        tft.setTextColor(GC9A01A_GREEN); tft.setTextSize(2);
        tft.setCursor(72, 110); tft.print("GANASTE!");
    }
}

// =====================================================
// DRAW FLAPPY
// =====================================================

void ScreenManager::drawFlappy(int birdY, int pipeX, int gapY, int score,
                                bool gameOver, bool started) {
    const int OFX = 15, OFY = 22;
    const int W = 210, H = 195;
    const int PIPE_W = 18, GAP = 52;
    const int BIRD_X = 38, BIRD_R = 5;

    tft.fillScreen(GC9A01A_BLACK);

    // Sky gradient hint (top bar)
    tft.fillRect(OFX, OFY, W, 20, 0x0319); // dark blue

    // Pipes
    uint16_t pipeColor = 0x0400; // dark green
    if (pipeX >= 0 && pipeX < W) {
        // Top pipe
        tft.fillRect(OFX + pipeX, OFY, PIPE_W, gapY, pipeColor);
        tft.fillRect(OFX + pipeX - 2, OFY + gapY - 6, PIPE_W + 4, 6, GC9A01A_GREEN);
        // Bottom pipe
        int botY = gapY + GAP;
        tft.fillRect(OFX + pipeX, OFY + botY, PIPE_W, H - botY, pipeColor);
        tft.fillRect(OFX + pipeX - 2, OFY + botY, PIPE_W + 4, 6, GC9A01A_GREEN);
    }

    // Ground
    tft.fillRect(OFX, OFY + H - 8, W, 8, 0x4A00); // brown

    // Bird (yellow circle with beak)
    tft.fillCircle(OFX + BIRD_X, OFY + birdY, BIRD_R, GC9A01A_YELLOW);
    tft.fillRect(OFX + BIRD_X + BIRD_R - 1, OFY + birdY - 1, 4, 3, GC9A01A_ORANGE);

    // Score
    tft.setTextColor(GC9A01A_WHITE); tft.setTextSize(2);
    char buf[8]; sprintf(buf, "%d", score);
    int16_t x1, y1; uint16_t w, h;
    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(120 - w / 2, OFY + 4);
    tft.print(buf);

    if (!started) {
        tft.setTextColor(GC9A01A_YELLOW); tft.setTextSize(1);
        tft.setCursor(OFX + 50, OFY + H / 2 + 20); tft.print("[OK] para volar");
    }
    if (gameOver) {
        tft.setTextColor(GC9A01A_RED); tft.setTextSize(2);
        tft.setCursor(68, 120); tft.print("CRASH!");
        tft.setTextSize(1); tft.setTextColor(GC9A01A_WHITE);
        tft.setCursor(60, 140); tft.print("[OK] reiniciar");
    }
}

// =====================================================
// DRAW INVADERS
// =====================================================

void ScreenManager::drawInvaders(const bool inv[][6], int rows, int cols,
                                  int invOfsX, int invOfsY, int spacing,
                                  int shipX, int score,
                                  int bulletX, int bulletY, bool bulletActive,
                                  int invBulletX, int invBulletY, bool invBulletActive,
                                  bool gameOver, bool won) {
    const int OFX = 20, OFY = 25;
    const int INV_W = 12, INV_H = 8;
    const int SHIP_W = 14, SHIP_Y = 170;

    tft.fillScreen(GC9A01A_BLACK);

    // Score
    tft.setTextColor(GC9A01A_WHITE); tft.setTextSize(1);
    char buf[20]; sprintf(buf, "Puntos: %d", score);
    tft.setCursor(OFX + 40, OFY - 14); tft.print(buf);

    // Invaders
    static const uint16_t INV_COLORS[] = {GC9A01A_RED, GC9A01A_MAGENTA, GC9A01A_CYAN};
    for (int r = 0; r < rows; r++) {
        uint16_t c16 = INV_COLORS[r % 3];
        for (int c = 0; c < cols; c++) {
            if (!inv[r][c]) continue;
            int ix = OFX + invOfsX + c * spacing - INV_W / 2;
            int iy = OFY + invOfsY + r * 22 - INV_H / 2;
            tft.fillRect(ix, iy, INV_W, INV_H, c16);
            // Antenas
            tft.drawPixel(ix + 2, iy - 2, c16);
            tft.drawPixel(ix + INV_W - 3, iy - 2, c16);
        }
    }

    // Player ship (triangle-ish)
    tft.fillTriangle(
        OFX + shipX, OFY + SHIP_Y - 10,
        OFX + shipX - SHIP_W / 2, OFY + SHIP_Y,
        OFX + shipX + SHIP_W / 2, OFY + SHIP_Y,
        GC9A01A_GREEN
    );

    // Base line
    tft.drawFastHLine(OFX, OFY + SHIP_Y + 2, 200, GC9A01A_DARKGREY);

    // Bullets
    if (bulletActive)
        tft.fillRect(OFX + bulletX - 1, OFY + bulletY - 4, 2, 8, GC9A01A_WHITE);
    if (invBulletActive)
        tft.fillRect(OFX + invBulletX - 1, OFY + invBulletY - 4, 2, 8, GC9A01A_RED);

    if (gameOver) {
        tft.setTextColor(GC9A01A_RED); tft.setTextSize(2);
        tft.setCursor(60, 105); tft.print("GAME OVER");
    }
    if (won) {
        tft.setTextColor(GC9A01A_GREEN); tft.setTextSize(2);
        tft.setCursor(66, 105); tft.print("GANASTE!");
    }
}

// =====================================================
// DRAW MINESWEEPER
// =====================================================

void ScreenManager::drawMinesweeper(const bool revealed[][9], const bool flagged[][9],
                                     const bool mine[][9], const int adj[][9],
                                     int rows, int cols, int cx, int cy,
                                     int flagsLeft, int state) {
    const int CELL = 22;
    const int OFX = (240 - cols * CELL) / 2;
    const int OFY = 28;

    static const uint16_t ADJ_COLORS[] = {
        GC9A01A_BLACK,   // 0 (no se muestra)
        GC9A01A_BLUE,    // 1
        GC9A01A_GREEN,   // 2
        GC9A01A_RED,     // 3
        0x000F,          // 4 dark blue
        0x7800,          // 5 dark red
        GC9A01A_CYAN,    // 6
        GC9A01A_WHITE,   // 7
        GC9A01A_DARKGREY // 8
    };

    tft.fillScreen(GC9A01A_BLACK);

    // Header
    tft.setTextColor(GC9A01A_WHITE); tft.setTextSize(1);
    char buf[20]; sprintf(buf, "Minas: %d", flagsLeft);
    tft.setCursor(10, 10); tft.print(buf);
    if (state == 1) { tft.setTextColor(GC9A01A_GREEN); tft.setCursor(140, 10); tft.print("GANASTE!"); }
    if (state == 2) { tft.setTextColor(GC9A01A_RED);   tft.setCursor(140, 10); tft.print("BOOM!"); }

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int x = OFX + c * CELL;
            int y = OFY + r * CELL;
            bool isCursor = (c == cx && r == cy);

            if (revealed[r][c]) {
                tft.fillRect(x + 1, y + 1, CELL - 2, CELL - 2, 0x2945); // medium grey
                if (mine[r][c]) {
                    tft.fillCircle(x + CELL/2, y + CELL/2, 5, GC9A01A_RED);
                } else if (adj[r][c] > 0) {
                    tft.setTextColor(ADJ_COLORS[adj[r][c]]);
                    tft.setTextSize(1);
                    tft.setCursor(x + CELL/2 - 3, y + CELL/2 - 4);
                    tft.print(adj[r][c]);
                }
            } else if (flagged[r][c]) {
                tft.fillRect(x + 1, y + 1, CELL - 2, CELL - 2, 0x4208); // dark grey
                tft.fillRect(x + CELL/2 - 2, y + 4, 4, 10, GC9A01A_RED); // flag pole + flag
                tft.fillTriangle(x + CELL/2 + 1, y + 4, x + CELL/2 + 1, y + 10, x + CELL/2 + 7, y + 7, GC9A01A_RED);
            } else {
                uint16_t fillC = isCursor ? 0x630C : 0x4A49; // highlight cursor
                tft.fillRect(x + 1, y + 1, CELL - 2, CELL - 2, fillC);
            }

            // Grid lines
            tft.drawRect(x, y, CELL, CELL, GC9A01A_BLACK);
        }
    }

    // Controls hint
    tft.setTextColor(0x39C7); tft.setTextSize(1);
    tft.setCursor(15, 225); tft.print("[OK]rev [A]flag");
}

// =====================================================
// DRAW DOOM  (raycaster)
// =====================================================

void ScreenManager::drawDoom(float px, float py, float angle,
                              int health, int ammo, int kills,
                              bool isShooting, bool gameOver, bool allDead,
                              const DoomGame::Enemy* enemies, int numEnemies) {
    const int VIEW_W = 240;
    const int VIEW_H = 155; // 3D view height
    const int HUD_Y  = VIEW_H;
    const float FOV_HALF = 0.55f; // ~63 degree FOV plane factor

    // Ceiling and floor
    tft.fillRect(0, 0, VIEW_W, VIEW_H / 2, 0x0319);    // dark blue ceiling
    tft.fillRect(0, VIEW_H / 2, VIEW_W, VIEW_H / 2, 0x2945); // grey floor

    float dirX = cosf(angle), dirY = sinf(angle);
    float plnX = -dirY * FOV_HALF, plnY = dirX * FOV_HALF;

    // Raycasting — one column at a time
    for (int col = 0; col < VIEW_W; col++) {
        float camX = 2.0f * col / (float)VIEW_W - 1.0f;
        float rayDirX = dirX + plnX * camX;
        float rayDirY = dirY + plnY * camX;

        int mapX = (int)px, mapY = (int)py;

        float deltaDX = (fabsf(rayDirX) < 1e-6f) ? 1e6f : fabsf(1.0f / rayDirX);
        float deltaDY = (fabsf(rayDirY) < 1e-6f) ? 1e6f : fabsf(1.0f / rayDirY);

        float sideDistX, sideDistY;
        int stepX, stepY;

        if (rayDirX < 0) { stepX = -1; sideDistX = (px - mapX) * deltaDX; }
        else             { stepX =  1; sideDistX = (mapX + 1.0f - px) * deltaDX; }
        if (rayDirY < 0) { stepY = -1; sideDistY = (py - mapY) * deltaDY; }
        else             { stepY =  1; sideDistY = (mapY + 1.0f - py) * deltaDY; }

        bool hit = false; int side = 0;
        for (int step = 0; step < 20 && !hit; step++) {
            if (sideDistX < sideDistY) { sideDistX += deltaDX; mapX += stepX; side = 0; }
            else                        { sideDistY += deltaDY; mapY += stepY; side = 1; }
            if (mapX >= 0 && mapX < DoomGame::MAP_W && mapY >= 0 && mapY < DoomGame::MAP_H)
                if (DoomGame::MAP[mapY][mapX] == 1) hit = true;
        }

        if (!hit) continue;

        float perpWallDist = (side == 0)
            ? (sideDistX - deltaDX)
            : (sideDistY - deltaDY);
        if (perpWallDist < 0.01f) perpWallDist = 0.01f;

        int lineH = (int)(VIEW_H / perpWallDist);
        int drawStart = max(0, VIEW_H / 2 - lineH / 2);
        int drawEnd   = min(VIEW_H - 1, VIEW_H / 2 + lineH / 2);

        // Wall color: side 0 = lighter, side 1 = darker; closer = brighter
        float bright = 1.0f / (perpWallDist * 0.4f + 0.5f);
        if (bright > 1.0f) bright = 1.0f;
        if (side == 1) bright *= 0.6f;

        uint8_t r8 = (uint8_t)(bright * 180);
        uint8_t g8 = (uint8_t)(bright * 80);
        uint8_t b8 = (uint8_t)(bright * 60);
        uint16_t wallColor = tft.color565(r8, g8, b8);

        tft.drawFastVLine(col, drawStart, drawEnd - drawStart + 1, wallColor);
    }

    // Enemy sprites (simple billboard: project to screen)
    for (int i = 0; i < numEnemies; i++) {
        if (!enemies[i].alive) continue;
        float spX = enemies[i].x - px;
        float spY = enemies[i].y - py;
        float invDet = 1.0f / (plnX * dirY - dirX * plnY);
        float transX =  invDet * (dirY * spX - dirX * spY);
        float transY =  invDet * (-plnY * spX + plnX * spY);
        if (transY <= 0.1f) continue;

        int sprScreenX = (int)(VIEW_W / 2.0f * (1.0f + transX / transY));
        int sprH = abs((int)(VIEW_H / transY));
        int sprW = sprH;
        int drawSX = max(0, sprScreenX - sprW / 2);
        int drawEX = min(VIEW_W - 1, sprScreenX + sprW / 2);
        int drawSY = max(0, VIEW_H / 2 - sprH / 2);
        int drawEY = min(VIEW_H - 1, VIEW_H / 2 + sprH / 2);

        // Simple sprite: magenta rectangle
        float brightSp = 1.0f / (transY * 0.5f + 0.5f);
        if (brightSp > 1.0f) brightSp = 1.0f;
        uint16_t spColor = tft.color565((uint8_t)(brightSp * 200), 0, (uint8_t)(brightSp * 200));
        tft.fillRect(drawSX, drawSY, drawEX - drawSX, drawEY - drawSY, spColor);
    }

    // Crosshair
    tft.drawFastHLine(115, VIEW_H / 2, 10, GC9A01A_WHITE);
    tft.drawFastVLine(120, VIEW_H / 2 - 5, 10, GC9A01A_WHITE);

    // Gun sprite (when shooting: bigger)
    int gunW = isShooting ? 50 : 40;
    int gunH = isShooting ? 35 : 28;
    tft.fillRect(120 - gunW / 2, VIEW_H - gunH, gunW, gunH,
                 isShooting ? GC9A01A_ORANGE : GC9A01A_DARKGREY);
    tft.fillRect(120 - 4, VIEW_H - gunH - 8, 8, 10, 0x4208); // barrel

    // HUD bar
    tft.fillRect(0, HUD_Y, 240, 240 - HUD_Y, GC9A01A_BLACK);
    tft.drawFastHLine(0, HUD_Y, 240, GC9A01A_DARKGREY);

    // Health bar
    tft.setTextColor(GC9A01A_RED); tft.setTextSize(1);
    tft.setCursor(4, HUD_Y + 5); tft.print("HP");
    int hpBarW = (health * 60) / 100;
    tft.fillRect(20, HUD_Y + 4, hpBarW, 8, GC9A01A_RED);
    tft.drawRect(20, HUD_Y + 4, 60, 8, GC9A01A_DARKGREY);

    // Ammo
    tft.setTextColor(GC9A01A_YELLOW); tft.setTextSize(1);
    char buf[24];
    sprintf(buf, "AMMO:%d", ammo);
    tft.setCursor(90, HUD_Y + 5); tft.print(buf);

    // Kills
    tft.setTextColor(GC9A01A_CYAN); tft.setTextSize(1);
    sprintf(buf, "K:%d/5", kills);
    tft.setCursor(160, HUD_Y + 5); tft.print(buf);

    // Controls hint (bottom)
    tft.setTextColor(0x39C7); tft.setTextSize(1);
    tft.setCursor(10, HUD_Y + 18);
    tft.print("^v mov <> giro [A]disp");

    if (gameOver) {
        tft.fillRect(40, 60, 160, 40, GC9A01A_BLACK);
        tft.setTextColor(GC9A01A_RED); tft.setTextSize(2);
        tft.setCursor(52, 70); tft.print("YOU DIED");
        tft.setTextColor(GC9A01A_WHITE); tft.setTextSize(1);
        tft.setCursor(55, 92); tft.print("[OK] reiniciar");
    }
    if (allDead) {
        tft.fillRect(40, 60, 160, 40, GC9A01A_BLACK);
        tft.setTextColor(GC9A01A_GREEN); tft.setTextSize(2);
        tft.setCursor(50, 70); tft.print("VICTORIA");
        tft.setTextColor(GC9A01A_WHITE); tft.setTextSize(1);
        tft.setCursor(55, 92); tft.print("[OK] reiniciar");
    }
}
