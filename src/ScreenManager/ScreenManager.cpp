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

void ScreenManager::drawSynth(int noteIdx, int octave, bool playing, bool sustain) {
    tft.fillScreen(GC9A01A_BLACK);

    int16_t x1, y1;
    uint16_t w, h;

    // --- Título ---
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_CYAN);
    tft.getTextBounds("SYNTH", 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 10);
    tft.print("SYNTH");

    // --- Anillo de color si está sonando ---
    int cx = 120, cy = 112;
    if (playing) {
        uint16_t rc = SYNTH_KEY_COLORS[noteIdx % 12];
        tft.drawCircle(cx, cy, 58, rc);
        tft.drawCircle(cx, cy, 59, rc);
        tft.drawCircle(cx, cy, 60, rc);
    }

    // --- Nombre de nota grande en el centro ---
    char noteBuf[5];
    snprintf(noteBuf, sizeof(noteBuf), "%s%d", SYNTH_NOTE_NAMES[noteIdx], octave);
    tft.setTextSize(4);
    uint16_t noteColor = playing ? SYNTH_KEY_COLORS[noteIdx % 12] : GC9A01A_WHITE;
    tft.setTextColor(noteColor);
    tft.getTextBounds(noteBuf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, cy - h / 2);
    tft.print(noteBuf);

    // --- Indicador sustain ---
    if (sustain) {
        tft.setTextSize(1);
        tft.setTextColor(GC9A01A_YELLOW);
        tft.setCursor(10, 48);
        tft.print("SUSTAIN");
    }

    // --- Estado ---
    tft.setTextSize(1);
    const char* stStr = playing ? "TOCANDO" : "LISTO";
    uint16_t stColor  = playing ? GC9A01A_GREEN : GC9A01A_DARKGREY;
    tft.setTextColor(stColor);
    tft.getTextBounds(stStr, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((240 - w) / 2, 156);
    tft.print(stStr);

    // --- Teclado de 12 teclas (una octava) ---
    // 12 teclas × 14px + 11 gaps × 1px = 179px → desde x=30
    const int KX = 31, KY = 168, KW = 13, KH = 28, KGAP = 1;
    for (int i = 0; i < 12; i++) {
        int kx = KX + i * (KW + KGAP);
        bool isActive = (i == noteIdx);
        uint16_t col;
        if (isActive) {
            col = SYNTH_KEY_COLORS[i];
        } else if (SYNTH_IS_BLACK[i]) {
            col = 0x4208; // gris oscuro
        } else {
            col = GC9A01A_WHITE;
        }
        tft.fillRect(kx, KY, KW, KH, col);
        if (!isActive) tft.drawRect(kx, KY, KW, KH, GC9A01A_DARKGREY);
    }

    // --- Instrucciones ---
    tft.setTextSize(1);
    tft.setTextColor(0x4208);
    tft.setCursor(24, 202);
    tft.print("<> Nota  ^v Oct  [A] Sus");
    tft.setCursor(44, 213);
    tft.print("[OK] Toca  [B] Silencio");
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
// ---------------------------------------------------------------------------
// drawBluetooth
// ---------------------------------------------------------------------------
void ScreenManager::drawBluetooth(bool connected, const char* device) {
    tft.fillScreen(GC9A01A_BLACK);

    const int CX = 120;
    int16_t x1; int16_t y1; uint16_t w; uint16_t h;

    // Titulo
    tft.setTextSize(1);
    tft.setTextColor(0x039F);  // azul BT
    const char* hdr = "BLUETOOTH REMOTE";
    tft.getTextBounds(hdr, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(CX - w / 2, 22);
    tft.print(hdr);

    // Icono / estado
    tft.setTextSize(3);
    tft.setTextColor(connected ? GC9A01A_CYAN : GC9A01A_DARKGREY);
    const char* icon = "BT";
    tft.getTextBounds(icon, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(CX - w / 2, 50);
    tft.print(icon);

    // Nombre del dispositivo
    tft.setTextSize(1);
    tft.setTextColor(connected ? GC9A01A_GREEN : GC9A01A_DARKGREY);
    tft.getTextBounds(device, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(CX - w / 2, 90);
    tft.print(device);

    tft.drawFastHLine(30, 105, 180, GC9A01A_DARKGREY);

    if (connected) {
        // Instrucciones controles
        tft.setTextColor(GC9A01A_LIGHTGREY);
        struct { const char* label; int y; } rows[] = {
            {"[OK]  Play / Pause",  118},
            {"[<]   Anterior",      132},
            {"[>]   Siguiente",     146},
            {"[^]   Volumen +",     160},
            {"[v]   Volumen -",     174},
        };
        for (auto& r : rows) {
            tft.getTextBounds(r.label, 0, 0, &x1, &y1, &w, &h);
            tft.setCursor(CX - w / 2, r.y);
            tft.print(r.label);
        }
    } else {
        tft.setTextColor(GC9A01A_DARKGREY);
        const char* hint = "Conecta 'ESP32-Remote'";
        tft.getTextBounds(hint, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor(CX - w / 2, 120);
        tft.print(hint);
        const char* hint2 = "en Bluetooth de tu cel";
        tft.getTextBounds(hint2, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor(CX - w / 2, 134);
        tft.print(hint2);
    }
}
