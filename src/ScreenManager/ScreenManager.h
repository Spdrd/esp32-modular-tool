#pragma once
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <TJpg_Decoder.h>
#include "../DoomGame/DoomGame.h"
#include "../EspNowLedManager/EspNowLedManager.h"

struct ScreenPinConfig
{
    int tftCsPin;
    int tftRstPin;
    int tftMisoPin;
    int tftDcPin;
    int tftMosiPin;
    int tftSclkPin;
};

// Vista del synth (selector circular + secuenciador en bucle)
struct SynthView {
    int  noteIdx;      // 0-11 nota seleccionada
    int  octave;       // octava mostrada
    bool playing;      // nota en preview sonando
    int  mode;         // 0 = TOCAR, 1 = SECUENCIA
    const int* seqNotes;  // notas de los pasos
    const int* seqOcts;   // octavas de los pasos (no usadas en dibujo, reservado)
    const int* seqDurs;   // duracion de cada paso en tiempos (beats)
    int  seqLen;       // numero de pasos
    int  seqCursor;    // paso seleccionado (modo SECUENCIA)
    bool seqPlaying;   // bucle activo
    int  playStep;     // paso sonando ahora (-1 ninguno)
    int  tempoBpm;     // tempo del bucle
};

class ScreenManager {

public:

    ScreenManager(ScreenPinConfig config);

    void begin();
    void update(String text);
    void clear();

    // Retorna true si el frame parece valido, false si es mayormente negro (basura).
    bool showJpeg(uint8_t* jpgBuffer, uint32_t jpgSize, int jpg_w, int jpg_h);

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

    void drawTetris(const uint8_t grid[20][10], const int8_t nextCells[4][2],
                    int nextType, int score, int level, bool gameOver,
                    int heldType, const int8_t heldCells[4][2]);

    void drawGame2048(const int grid[4][4], int score, bool won, bool gameOver);

    void drawMorse(const char* pattern, const char* output, char lastDecoded);

    void drawMusicPlayer(const char* name, int noteIdx, int total,
                         bool playing, bool paused);

    void drawSynth(const SynthView& v);

    void drawLinterna(uint8_t colorIndex, uint8_t brightness, bool on);

    void drawSirena(const char* pattern, bool on, uint8_t brightness);

    // Pantalla de control del Cam Car: si hasVideo, dibuja el frame JPEG
    // recibido a pantalla completa con overlay de comando; si no, una
    // vista de estado simple con icono de direccion.
    void drawCamCarStatus(bool active, const char* cmdName, bool linked);
    void drawCamCarOverlay(const char* cmdName, bool linked, uint32_t fps);

    // Teclado BLE: pressedMask bits 0=W 1=A 2=S 3=D 4=ESC 5=ENTER 6=SPACE
    void drawBtKeyboard(bool active, bool connected, const char* deviceName,
                        uint8_t pressedMask);

    // Control multimedia BLE. lastAction = etiqueta transitoria del ultimo
    // comando enviado ("" si no hay ninguno reciente).
    void drawMusicControl(bool active, bool connected, const char* deviceName,
                          bool playing, const char* lastAction);

    // Estado del joystick: posicion analogica + direcciones discretas activas.
    void drawJoystick(int rawX, int rawY, int centerX, int centerY,
                      float x, float y, float magnitude, float angle,
                      bool up, bool down, bool left, bool right);

    // Tira LED por ESP-NOW. param = fila seleccionada (0..5):
    // 0 efecto, 1 brillo, 2 rojo, 3 verde, 4 azul, 5 velocidad.
    void drawLedStrip(int param, uint8_t effect, const char* effectName,
                      uint8_t brightness, uint8_t r, uint8_t g, uint8_t b,
                      uint8_t bps, bool active, bool lastOk, bool broadcast,
                      uint32_t sent, uint32_t failed);

    // ---- Nuevos juegos ----
    void drawPong(int ballX, int ballY, int playerY, int aiY,
                  int pScore, int aScore, bool gameOver, bool playerWon);

    void drawBreakout(const bool bricks[][7], int rows, int cols,
                      int brickW, int brickH, int padX, int ballX, int ballY,
                      int score, int lives, bool launched, bool gameOver, bool won);

    void drawFlappy(int birdY, int pipeX, int gapY, int score, bool gameOver, bool started);

    void drawInvaders(const bool inv[][6], int rows, int cols,
                      int invOfsX, int invOfsY, int spacing,
                      int shipX, int score,
                      int bulletX, int bulletY, bool bulletActive,
                      int invBulletX, int invBulletY, bool invBulletActive,
                      bool gameOver, bool won);

    void drawMinesweeper(const bool revealed[][9], const bool flagged[][9],
                         const bool mine[][9], const int adj[][9],
                         int rows, int cols, int cx, int cy,
                         int flagsLeft, int state);

    void drawDoom(float px, float py, float angle,
                  int health, int ammo, int kills,
                  bool isShooting, bool gameOver, bool allDead,
                  const DoomGame::Enemy* enemies, int numEnemies);

    Adafruit_GC9A01A tft;

private:

    ScreenPinConfig config;
};
