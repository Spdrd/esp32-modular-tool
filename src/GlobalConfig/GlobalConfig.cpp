#include "GlobalConfig/GlobalConfig.h"

// =====================================================
// PINS
// =====================================================

static ScreenPinConfig screenPins = {
    .tftCsPin   = TFT_CS,
    .tftRstPin  = TFT_RST,
    .tftMisoPin = TFT_MISO,
    .tftDcPin   = TFT_DC,
    .tftMosiPin = TFT_MOSI,
    .tftSclkPin = TFT_SCLK
};

ButtonPinConfig buttonConfig = {
    .bMenuPin   = BTN_MENU,
    .bAPin      = BTN_A,
    .bBPin      = BTN_B,
    .bUpPin     = BTN_UP,
    .bDownPin   = BTN_DOWN,
    .bLeftPin   = BTN_LEFT,
    .bRightPin  = BTN_RIGHT,
    .bOkPin     = BTN_OK
};

ButtonManager buttons(buttonConfig);
ScreenManager screen(screenPins);
SnakeGame snake;
CronoManager crono;
DiceManager dice;
CanvasManager canvas;
TimerManager timer;
SimonGame simon;
TetrisGame tetris;
Game2048 game2048;
MorseCode morse;
SpeakerManager speaker(SPEAKER_PIN, SPEAKER_CHANNEL);
LinternaManager linterna;

static EspNowPeerConfig camCarConfig = { .mac = CAMCAR_MAC };
EspNowManager camCar(camCarConfig);

// =====================================================
// MENU DATA
// =====================================================

static const MenuItem testItems[] = {
    {"Menu",   enterTestMenu},
    {"A",      enterTestA},
    {"B",      enterTestB},
    {"UP",     enterTestUp},
    {"DOWN",   enterTestDown},
    {"LEFT",   enterTestLeft},
    {"RIGHT",  enterTestRight},
    {"OK",     enterTestOk},
    {"All",    enterTestAll},
};

static const MenuItem displayItems[] = {
    {"Rojo",   enterFillRed},
    {"Verde",  enterFillGreen},
    {"Azul",   enterFillBlue},
    {"Limpiar",enterClear},
};

static const MenuItem infoItems[] = {
    {"Version 1.0", enterVersion},
    {"Creditos",    enterCredits},
};

static const MenuItem gamesItems[] = {
    {"Snake",  enterSnake},
    {"Simon",  enterSimon},
    {"Tetris", enterTetris},
    {"2048",   enterGame2048},
};

static const MenuItem toolsItems[] = {
    {"Cronometro",  enterCronometro},
    {"Dado",        enterDado},
    {"Canvas",      enterCanvas},
    {"Temporizador",enterTimer},
    {"Morse",       enterMorse},
    {"Synth",       enterSynth},
    {"Linterna",    enterLinterna},
    {"Sirena",      enterSirena},
};

static const MenuItem musicItems[] = {
    {"Aerodynamic",       []() { enterSong(SONG_AERODYNAMIC); }},
    {"Get Lucky",         []() { enterSong(SONG_GETLUCKY); }},
    {"H-B-F-S",           []() { enterSong(SONG_HBF); }},
    {"Never Gonna",       []() { enterSong(SONG_NGGYU); }},
    {"Kids",              []() { enterSong(SONG_KIDS); }},
    {"Adults Are Talking",[]() { enterSong(SONG_ADULTS_ARE_TALKING); }},
    {"Outer Wilds",       []() { enterSong(SONG_OUTER_WILDS); }},
    {"Riptide",           []() { enterSong(SONG_RIPTIDE); }},
};

static const MenuItem devicesItems[] = {
    {"Cam Car", enterCamCar},
};

static const MenuSection menuSections[] = {
    {"Tests",      testItems,      9},
    {"Pantalla",   displayItems,   4},
    {"Info",       infoItems,      2},
    {"Juegos",     gamesItems,     4},
    {"Herramientas",toolsItems,    8},
    {"Musica",     musicItems,     8},
    {"Mis Dispositivos", devicesItems, 1},
};

const MenuSection* sections = menuSections;
int sectionCount = sizeof(menuSections) / sizeof(menuSections[0]);

int currentSection = 0;
int currentItem    = 0;
static void (*itemLoopCallback)() = nullptr;

// =====================================================
// SIMON STATE
// =====================================================

static const uint16_t SIMON_NOTES[4] = { E5, C5, G4, A4 };
static MusNote s_simonNote[1];
static Song    s_simonSong = { "Simon", s_simonNote, 1 };

// =====================================================
// SYNTH STATE
// =====================================================

static const uint16_t SYNTH_FREQS[4][12] = {
    {131,139,147,156,165,175,185,196,208,220,233,247},
    {262,277,294,311,330,349,370,392,415,440,466,494},
    {523,554,587,622,659,698,740,784,831,880,932,988},
    {1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976},
};
static int  s_synthNote    = 0;
static int  s_synthOct     = 1;     // indice 0..3 en SYNTH_FREQS
static bool s_synthPlaying = false;
static bool s_synthOkWas   = false;
static int  s_synthMode    = 0;     // 0 = TOCAR, 1 = SECUENCIA

// --- Secuenciador ---
static const int SYNTH_MAX_STEPS = 8;
static const int SYNTH_MAX_DUR   = 8;   // tiempos maximos por nota
static int  s_seqNote[SYNTH_MAX_STEPS];
static int  s_seqOct[SYNTH_MAX_STEPS];
static int  s_seqDur[SYNTH_MAX_STEPS];  // duracion en tiempos de cada paso
static int  s_seqLen        = 0;
static int  s_seqCursor     = 0;
static bool s_seqPlaying    = false;
static int  s_seqPlayStep   = -1;
static int  s_seqBpm        = 120;
static unsigned long s_seqStepEndMs = 0;
static unsigned long s_seqNoteOffMs = 0;

// =====================================================
// ON ENTER FUNCTIONS
// =====================================================

static void showMessageStay(const char* msg, uint16_t color) {
    screen.showTextLines(msg, "", "", color);
}

void enterTestMenu()  { showMessageStay("Menu",  GC9A01A_BLUE);    }
void enterTestA()     { showMessageStay("A",     GC9A01A_GREEN);   }
void enterTestB()     { showMessageStay("B",     GC9A01A_RED);     }
void enterTestUp()    { showMessageStay("UP",    GC9A01A_CYAN);    }
void enterTestDown()  { showMessageStay("DOWN",  GC9A01A_YELLOW);  }
void enterTestLeft()  { showMessageStay("LEFT",  GC9A01A_MAGENTA); }
void enterTestRight() { showMessageStay("RIGHT", GC9A01A_WHITE);   }
void enterTestOk()    { showMessageStay("OK",    GC9A01A_GREEN);   }
void enterTestAll()   { showMessageStay("ALL",   GC9A01A_ORANGE);  }

void enterFillRed()   { screen.tft.fillScreen(GC9A01A_RED);   }
void enterFillGreen() { screen.tft.fillScreen(GC9A01A_GREEN); }
void enterFillBlue()  { screen.tft.fillScreen(GC9A01A_BLUE);  }
void enterClear()     { screen.tft.fillScreen(GC9A01A_BLACK); }

void enterVersion()   { showMessageStay("v1.0 - 2026", GC9A01A_CYAN); }
void enterCredits()   { showMessageStay("By J. Diego", GC9A01A_BLUE); }

// =====================================================
// SIMON
// =====================================================

static void simonPlayBtn(int btn) {
    s_simonNote[0] = { SIMON_NOTES[btn], 200, 0 }; // 200ms = quarter @ 300 BPM
    speaker.play(&s_simonSong, false);
}

static void simonPlayLose() {
    s_simonNote[0] = { E3, 400, 0 }; // 400ms = half @ 300 BPM
    speaker.play(&s_simonSong, false);
}

static void drawSimonState() {
    screen.drawSimon(
        simon.getHighlight(),
        simon.getScore(),
        (int)simon.getState(),
        simon.isFlashOn()
    );
}

static int s_simonLastHighlight = -1;

static void simonLoop() {
    speaker.update();
    if (simon.update()) {
        drawSimonState();

        int hl = simon.getHighlight();
        SimonGame::State st = simon.getState();

        // Suena cuando se ilumina un color durante SHOWING
        if (st == SimonGame::SHOWING && hl >= 0 && hl != s_simonLastHighlight) {
            simonPlayBtn(hl);
        }
        // Suena la nota grave al perder
        if (st == SimonGame::LOSE && s_simonLastHighlight != -2) {
            simonPlayLose();
            s_simonLastHighlight = -2; // marca para no repetir
            return;
        }

        s_simonLastHighlight = hl;
    }
}

void enterSimon() {
    simon.reset();
    speaker.stop();
    s_simonLastHighlight = -1;
    drawSimonState();
    itemLoopCallback = simonLoop;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { if (simon.pressButton(0)) { simonPlayBtn(0); drawSimonState(); } };
    cbs.onDown  = []() { if (simon.pressButton(1)) { simonPlayBtn(1); drawSimonState(); } };
    cbs.onLeft  = []() { if (simon.pressButton(2)) { simonPlayBtn(2); drawSimonState(); } };
    cbs.onRight = []() { if (simon.pressButton(3)) { simonPlayBtn(3); drawSimonState(); } };
    cbs.onOk    = []() {
        if (simon.getState() == SimonGame::LOSE) {
            simon.reset();
            speaker.stop();
            s_simonLastHighlight = -1;
            drawSimonState();
        }
    };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// TETRIS
// =====================================================

static void drawTetrisState() {
    uint8_t grid[TetrisGame::ROWS][TetrisGame::COLS];
    tetris.getDisplayGrid(grid);
    int8_t nextCells[4][2];
    tetris.getNextCells(nextCells);
    int8_t heldCells[4][2];
    tetris.getHeldCells(heldCells);
    screen.drawTetris(grid, nextCells, tetris.getNextPiece(),
                      tetris.getScore(), tetris.getLevel(), tetris.isGameOver(),
                      tetris.getHeldPiece(), heldCells);
}

static void tetrisLoop() {
    if (tetris.update()) drawTetrisState();
}

void enterTetris() {
    tetris.reset();
    drawTetrisState();
    itemLoopCallback = tetrisLoop;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { tetris.hardDrop();   drawTetrisState(); };
    cbs.onDown  = []() { tetris.softDrop();   drawTetrisState(); };
    cbs.onLeft  = []() { tetris.moveLeft();   drawTetrisState(); };
    cbs.onRight = []() { tetris.moveRight();  drawTetrisState(); };
    cbs.onA     = []() {
        if (tetris.isGameOver()) { tetris.reset(); drawTetrisState(); }
        else { tetris.rotate();    drawTetrisState(); }
    };
    cbs.onB     = []() {
        if (!tetris.isGameOver()) { tetris.rotateCCW(); drawTetrisState(); }
    };
    cbs.onOk    = []() { tetris.holdPiece();  drawTetrisState(); };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// 2048
// =====================================================

static void draw2048State() {
    int grid[4][4];
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            grid[r][c] = game2048.getCell(r, c);
    screen.drawGame2048(grid, game2048.getScore(), game2048.isWon(), game2048.isGameOver());
}

void enterGame2048() {
    game2048.reset();
    draw2048State();
    itemLoopCallback = nullptr;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { if (game2048.slideUp())    draw2048State(); };
    cbs.onDown  = []() { if (game2048.slideDown())  draw2048State(); };
    cbs.onLeft  = []() { if (game2048.slideLeft())  draw2048State(); };
    cbs.onRight = []() { if (game2048.slideRight()) draw2048State(); };
    cbs.onOk    = []() {
        if (game2048.isGameOver() || game2048.isWon()) {
            game2048.reset();
            draw2048State();
        }
    };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// MORSE
// =====================================================

// Tiempos para interpretar pulsaciones de OK como morse
static const unsigned long MORSE_DOT_MAX_MS  = 300;   // < 300ms = punto
static const unsigned long MORSE_CHAR_GAP_MS = 1500;  // 1.5s sin pulsar = confirma letra
static const unsigned long MORSE_WORD_GAP_MS = 3000;  // 3s sin pulsar = espacio de palabra

// Estado de la pulsación OK
static bool          s_okWasDown    = false;
static unsigned long s_okPressStart = 0;
static unsigned long s_lastReleaseMs = 0;
static bool          s_hasPattern   = false; // hay símbolos sin confirmar

// Buffer de notas para reproducir el beep de cada símbolo
static MusNote s_morseNotes[MorseCode::MAX_PATTERN * 2];
static Song    s_morseSong = { "Morse", s_morseNotes, 0 };

static void playMorseSymbol(bool isDot) {
    s_morseNotes[0] = { A4, (uint16_t)(isDot ? 60 : 90), 0 }; // 60ms=dot, 90ms=dash @ 250 BPM
    s_morseSong.count = 1;
    speaker.play(&s_morseSong, false);
}

static void drawMorseState() {
    screen.drawMorse(morse.getPattern(), morse.getOutput(), morse.getLastDecoded());
}

static void morseLoop() {
    speaker.update();

    bool okDown = buttons.isOkDown();
    unsigned long now = millis();

    if (okDown && !s_okWasDown) {
        // Inicio de pulsación: para reproducción anterior si hubiese
        speaker.stop();
        s_okPressStart = now;
        s_okWasDown    = true;
    } else if (!okDown && s_okWasDown) {
        // Fin de pulsación: clasificar punto o raya
        bool isDot = (now - s_okPressStart) < MORSE_DOT_MAX_MS;
        if (isDot) morse.addDot(); else morse.addDash();
        s_hasPattern   = true;
        s_lastReleaseMs = now;
        s_okWasDown    = false;
        drawMorseState();
        playMorseSymbol(isDot);
    }

    // Timers de silencio (solo cuando hay patrón pendiente y OK no está presionado)
    if (s_hasPattern && !s_okWasDown && s_lastReleaseMs > 0) {
        unsigned long elapsed = now - s_lastReleaseMs;

        if (elapsed >= MORSE_WORD_GAP_MS) {
            // 3s sin pulsar: confirma letra + agrega espacio
            morse.confirmChar();
            morse.addSpace();
            s_hasPattern    = false;
            s_lastReleaseMs = 0;
            drawMorseState();
        } else if (elapsed >= MORSE_CHAR_GAP_MS) {
            // 1.5s sin pulsar: confirma letra
            morse.confirmChar();
            s_hasPattern    = false;
            s_lastReleaseMs = 0;
            drawMorseState();
        }
    }
}

void enterMorse() {
    morse.reset();
    speaker.stop();
    s_okWasDown     = false;
    s_okPressStart  = 0;
    s_lastReleaseMs = 0;
    s_hasPattern    = false;
    drawMorseState();
    itemLoopCallback = morseLoop;

    ButtonActionCallbacks cbs;
    cbs.onDown = []() { morse.deleteLast(); s_hasPattern = (morse.getPatternLen() > 0); drawMorseState(); };
    cbs.onUp   = []() { morse.clearPattern(); s_hasPattern = false; s_lastReleaseMs = 0; drawMorseState(); };
    cbs.onLeft = []() { morse.clearAll();    s_hasPattern = false; s_lastReleaseMs = 0; drawMorseState(); };
    cbs.onMenu = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// SYNTH
// =====================================================

static void drawSynthState() {
    SynthView v;
    v.noteIdx    = s_synthNote;
    v.octave     = s_synthOct + 3;
    v.playing    = s_synthPlaying;
    v.mode       = s_synthMode;
    v.seqNotes   = s_seqNote;
    v.seqOcts    = s_seqOct;
    v.seqDurs    = s_seqDur;
    v.seqLen     = s_seqLen;
    v.seqCursor  = s_seqCursor;
    v.seqPlaying = s_seqPlaying;
    v.playStep   = s_seqPlayStep;
    v.tempoBpm   = s_seqBpm;
    screen.drawSynth(v);
}

static void synthStartNote() {
    s_synthPlaying = true;
    speaker.playTone(SYNTH_FREQS[s_synthOct][s_synthNote]);
}

static void synthStopNote() {
    s_synthPlaying = false;
    speaker.stopTone();
}

// Reproduce la nota de un paso de la secuencia
static void seqPlayStepTone(int i) {
    speaker.playTone(SYNTH_FREQS[s_seqOct[i]][s_seqNote[i]]);
}

static void synthLoop() {
    // --- Modo TOCAR: mantener OK para sonar (solo si el bucle no manda) ---
    if (s_synthMode == 0 && !s_seqPlaying) {
        bool okDown = buttons.isOkDown();
        if (okDown && !s_synthOkWas) {
            synthStartNote();
            drawSynthState();
        } else if (!okDown && s_synthOkWas) {
            synthStopNote();
            drawSynthState();
        }
        s_synthOkWas = okDown;
    }

    // --- Bucle del secuenciador (corre en cualquier modo) ---
    if (s_seqPlaying && s_seqLen > 0) {
        unsigned long now = millis();
        unsigned long beatMs = 60000UL / (unsigned long)s_seqBpm;
        if (s_seqPlayStep < 0 || now >= s_seqStepEndMs) {
            s_seqPlayStep = (s_seqPlayStep + 1) % s_seqLen;
            unsigned long stepMs = beatMs * (unsigned long)s_seqDur[s_seqPlayStep];
            seqPlayStepTone(s_seqPlayStep);
            s_seqStepEndMs = now + stepMs;
            s_seqNoteOffMs = now + (stepMs * 5) / 6;  // pequeño silencio al final
            drawSynthState();
        } else if (s_seqNoteOffMs != 0 && now >= s_seqNoteOffMs) {
            speaker.stopTone();
            s_seqNoteOffMs = 0;
        }
    }
}

static void synthAddStep() {
    if (s_seqLen >= SYNTH_MAX_STEPS) return;
    s_seqNote[s_seqLen] = s_synthNote;
    s_seqOct[s_seqLen]  = s_synthOct;
    s_seqDur[s_seqLen]  = 1;            // por defecto 1 tiempo
    s_seqLen++;
    s_seqCursor = s_seqLen - 1;
}

static void synthDeleteStep() {
    if (s_seqLen == 0) return;
    for (int k = s_seqCursor; k < s_seqLen - 1; k++) {
        s_seqNote[k] = s_seqNote[k + 1];
        s_seqOct[k]  = s_seqOct[k + 1];
        s_seqDur[k]  = s_seqDur[k + 1];
    }
    s_seqLen--;
    if (s_seqLen == 0) {
        s_seqPlaying  = false;
        s_seqPlayStep = -1;
        s_seqCursor   = 0;
        speaker.stopTone();
    } else if (s_seqCursor >= s_seqLen) {
        s_seqCursor = s_seqLen - 1;
    }
}

static void synthToggleLoop() {
    if (s_seqLen == 0) return;
    s_seqPlaying = !s_seqPlaying;
    if (s_seqPlaying) {
        synthStopNote();           // corta preview
        s_synthOkWas    = false;
        s_seqPlayStep   = -1;
        s_seqStepEndMs  = 0;
        s_seqNoteOffMs  = 0;
    } else {
        speaker.stopTone();
        s_seqPlayStep = -1;
    }
}

void enterSynth() {
    s_synthNote    = 0;
    s_synthOct     = 1;
    s_synthPlaying = false;
    s_synthOkWas   = false;
    s_synthMode    = 0;
    s_seqLen       = 0;
    s_seqCursor    = 0;
    s_seqPlaying   = false;
    s_seqPlayStep  = -1;
    s_seqBpm       = 120;
    speaker.stop();
    drawSynthState();
    itemLoopCallback = synthLoop;

    ButtonActionCallbacks cbs;

    // Modos: 0 = TOCAR, 1 = TEMPO, 2 = SECUENCIA
    cbs.onRight = []() {
        if (s_synthMode == 0) {
            s_synthNote = (s_synthNote + 1) % 12;
            if (s_synthPlaying) synthStartNote();
        } else if (s_synthMode == 2 && s_seqLen > 0) {
            s_seqCursor = (s_seqCursor + 1) % s_seqLen;
        }
        drawSynthState();
    };
    cbs.onLeft = []() {
        if (s_synthMode == 0) {
            s_synthNote = (s_synthNote + 11) % 12;
            if (s_synthPlaying) synthStartNote();
        } else if (s_synthMode == 2 && s_seqLen > 0) {
            s_seqCursor = (s_seqCursor + s_seqLen - 1) % s_seqLen;
        }
        drawSynthState();
    };
    cbs.onUp = []() {
        if (s_synthMode == 0) {
            if (s_synthOct < 3) { s_synthOct++; if (s_synthPlaying) synthStartNote(); }
        } else if (s_synthMode == 1) {
            if (s_seqBpm < 300) s_seqBpm += 10;   // mas rapido
        } else if (s_seqLen > 0) {
            if (s_seqDur[s_seqCursor] < SYNTH_MAX_DUR) s_seqDur[s_seqCursor]++;
        }
        drawSynthState();
    };
    cbs.onDown = []() {
        if (s_synthMode == 0) {
            if (s_synthOct > 0) { s_synthOct--; if (s_synthPlaying) synthStartNote(); }
        } else if (s_synthMode == 1) {
            if (s_seqBpm > 40) s_seqBpm -= 10;    // mas lento
        } else if (s_seqLen > 0) {
            if (s_seqDur[s_seqCursor] > 1) s_seqDur[s_seqCursor]--;
        }
        drawSynthState();
    };
    // OK: en TEMPO y SECUENCIA arranca/detiene el bucle; en TOCAR lo maneja el loop (mantener)
    cbs.onOk = []() {
        if (s_synthMode != 0) { synthToggleLoop(); drawSynthState(); }
    };
    // A: cicla de modo  TOCAR -> TEMPO -> SECUENCIA  (reutiliza los botones)
    cbs.onA = []() {
        s_synthMode = (s_synthMode + 1) % 3;
        if (s_synthMode != 0) synthStopNote();  // al salir de TOCAR corta el preview
        drawSynthState();
    };
    // B: en TOCAR agrega la nota actual; en SECUENCIA borra el paso
    cbs.onB = []() {
        if (s_synthMode == 0)      synthAddStep();
        else if (s_synthMode == 2) synthDeleteStep();
        drawSynthState();
    };
    cbs.onMenu = []() { synthStopNote(); speaker.stopTone(); returnToMenu(); };
    buttons.setCallbacks(cbs);
}

// =====================================================
// SNAKE
// =====================================================

static void snakeUp()    { snake.setDirection(0, -1); }
static void snakeDown()  { snake.setDirection(0, 1); }
static void snakeLeft()  { snake.setDirection(-1, 0); }
static void snakeRight() { snake.setDirection(1, 0); }

static void drawSnakeState() {
    int bodyX[100], bodyY[100];
    int len = snake.getLength();
    for (int i = 0; i < len; i++) {
        bodyX[i] = snake.getBodyX(i);
        bodyY[i] = snake.getBodyY(i);
    }
    screen.drawSnake(bodyX, bodyY, len, snake.getFoodX(), snake.getFoodY(),
                     snake.getScore(), snake.isGameOver());
}

static void snakeOk() {
    if (snake.isGameOver()) {
        snake.reset();
        drawSnakeState();
    }
}

static void snakeLoop() {
    if (snake.update()) {
        drawSnakeState();
    }
}

void enterSnake() {
    snake.reset();
    drawSnakeState();
    itemLoopCallback = snakeLoop;

    ButtonActionCallbacks cbs;
    cbs.onUp    = snakeUp;
    cbs.onDown  = snakeDown;
    cbs.onLeft  = snakeLeft;
    cbs.onRight = snakeRight;
    cbs.onOk    = snakeOk;
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// CRONOMETRO
// =====================================================

static void drawCronoState() {
    unsigned long laps[10];
    int count = crono.getLapCount();
    for (int i = 0; i < count; i++) laps[i] = crono.getLap(i);
    screen.drawCronometro(crono.getElapsed(), crono.isRunning(), laps, count);
}

static void cronoLoop() {
    if (crono.shouldRender()) drawCronoState();
}

void enterCronometro() {
    crono.reset();
    drawCronoState();
    itemLoopCallback = cronoLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk   = []() { crono.toggle(); drawCronoState(); };
    cbs.onA    = []() { crono.markLap(); drawCronoState(); };
    cbs.onB    = []() { crono.reset(); drawCronoState(); };
    cbs.onMenu = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// DADO
// =====================================================

static void drawDiceState() {
    screen.drawDice(dice.getMax(), dice.getResult(), dice.hasRolled());
}

void enterDado() {
    drawDiceState();
    itemLoopCallback = nullptr;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { dice.increaseMax(); drawDiceState(); };
    cbs.onDown  = []() { dice.decreaseMax(); drawDiceState(); };
    cbs.onOk    = []() { dice.roll(); drawDiceState(); };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// CANVAS
// =====================================================

static void drawCanvasState() {
    screen.drawCanvas(canvas.getGrid(), canvas.getCols(), canvas.getRows(),
                      canvas.getCursorX(), canvas.getCursorY(),
                      canvas.getCurrentColor());
}

void enterCanvas() {
    drawCanvasState();
    itemLoopCallback = nullptr;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { canvas.moveCursor(0, -1); drawCanvasState(); };
    cbs.onDown  = []() { canvas.moveCursor(0, 1); drawCanvasState(); };
    cbs.onLeft  = []() { canvas.moveCursor(-1, 0); drawCanvasState(); };
    cbs.onRight = []() { canvas.moveCursor(1, 0); drawCanvasState(); };
    cbs.onOk    = []() { canvas.paint(); drawCanvasState(); };
    cbs.onA     = []() { canvas.nextColor(); drawCanvasState(); };
    cbs.onB     = []() { canvas.reset(); drawCanvasState(); };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// TEMPORIZADOR
// =====================================================

static void drawTimerState() {
    unsigned long rem = timer.getRemainingMs();
    int m = rem / 60000;
    int s = (rem % 60000) / 1000;
    screen.drawTimer(m, s, timer.getState(), timer.getField(), timer.isFlashOn());
}

static void timerLoop() {
    if (timer.update()) {
        drawTimerState();
    }
}

void enterTimer() {
    timer.reset();
    drawTimerState();
    itemLoopCallback = timerLoop;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { timer.up(); drawTimerState(); };
    cbs.onDown  = []() { timer.down(); drawTimerState(); };
    cbs.onLeft  = []() { timer.nextField(); drawTimerState(); };
    cbs.onRight = []() { timer.nextField(); drawTimerState(); };
    cbs.onOk    = []() {
        if (timer.getState() == TimerManager::DONE) {
            timer.reset();
        } else {
            timer.ok();
        }
        drawTimerState();
    };
    cbs.onMenu  = []() { timer.reset(); returnToMenu(); };
    buttons.setCallbacks(cbs);
}

// =====================================================
// MUSICA
// =====================================================

static void drawMusicState() {
    screen.drawMusicPlayer(
        speaker.getSongName(),
        speaker.getNoteIdx(),
        speaker.getNoteCount(),
        speaker.isPlaying(),
        speaker.isPaused()
    );
}

static void musicLoop() {
    if (speaker.update()) drawMusicState();
}

void enterSong(const Song& song) {
    speaker.play(&song);
    drawMusicState();
    itemLoopCallback = musicLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk   = []() { speaker.togglePause(); drawMusicState(); };
    cbs.onMenu = []() { speaker.stop(); returnToMenu(); };
    buttons.setCallbacks(cbs);
}


// =====================================================
// LINTERNA
// =====================================================

static void drawLinternaState() {
    screen.drawLinterna(
        linterna.getColorIndex(),
        linterna.getBrightness(),
        linterna.isOn()
    );
}

void enterLinterna() {
    linterna.turnOff();
    drawLinternaState();
    itemLoopCallback = nullptr;

    ButtonActionCallbacks cbs;
    cbs.onOk    = []() { linterna.toggle();       drawLinternaState(); };
    cbs.onRight = []() { linterna.nextColor();    drawLinternaState(); };
    cbs.onLeft  = []() { linterna.prevColor();    drawLinternaState(); };
    cbs.onUp    = []() { linterna.brightnessUp(); drawLinternaState(); };
    cbs.onDown  = []() { linterna.brightnessDown(); drawLinternaState(); };
    cbs.onMenu  = []() { linterna.turnOff(); returnToMenu(); };
    buttons.setCallbacks(cbs);
}

// =====================================================
// SIRENA  (bocina + LED)
// =====================================================

static const int   SIRENA_NUM_PATTERNS = 3;
static const char* SIRENA_NAMES[SIRENA_NUM_PATTERNS] = { "Policia", "Wail", "Yelp" };

static bool          s_sirenaOn       = false;
static int           s_sirenaPattern  = 0;
static uint8_t       s_sirenaBright   = 200;
static unsigned long s_sirenaT0       = 0;
static unsigned long s_sirenaLastSwap = 0;
static bool          s_sirenaPhase    = false;

static void drawSirenaState() {
    screen.drawSirena(SIRENA_NAMES[s_sirenaPattern], s_sirenaOn, s_sirenaBright);
}

static void sirenaAllOff() {
    speaker.stopTone();
    linterna.turnOff();
}

static void sirenaLoop() {
    if (!s_sirenaOn) return;
    unsigned long now = millis();

    if (s_sirenaPattern == 0) {
        // Policia: dos tonos alternados + azul/rojo
        if (now - s_sirenaLastSwap >= 450) {
            s_sirenaLastSwap = now;
            s_sirenaPhase = !s_sirenaPhase;
            speaker.playTone(s_sirenaPhase ? 700 : 500);
            if (s_sirenaPhase) linterna.showColor(255, 0, 0, s_sirenaBright);
            else               linterna.showColor(0, 0, 255, s_sirenaBright);
        }
        return;
    }

    // Wail / Yelp: barrido de frecuencia (lento / rapido)
    static unsigned long lastUpd = 0;
    if (now - lastUpd < 15) return;
    lastUpd = now;

    unsigned long period = (s_sirenaPattern == 1) ? 2400 : 500;
    unsigned long t = (now - s_sirenaT0) % period;
    float ph  = (float)t / (float)period;
    float tri = (ph < 0.5f) ? (ph * 2.0f) : (2.0f - ph * 2.0f);
    int freq  = 400 + (int)(tri * 700.0f);
    speaker.playTone((uint16_t)freq);

    if (s_sirenaPattern == 1) {
        // Wail: rojo con brillo pulsante
        uint8_t b = (uint8_t)(40 + tri * (float)(s_sirenaBright - 40));
        linterna.showColor(255, 0, 0, b);
    } else {
        // Yelp: rojo/blanco rapido
        if (tri > 0.5f) linterna.showColor(255, 255, 255, s_sirenaBright);
        else            linterna.showColor(255, 0,   0,   s_sirenaBright);
    }
}

static void sirenaToggle() {
    s_sirenaOn = !s_sirenaOn;
    if (s_sirenaOn) {
        s_sirenaT0       = millis();
        s_sirenaLastSwap = 0;
        s_sirenaPhase    = false;
    } else {
        sirenaAllOff();
    }
}

void enterSirena() {
    s_sirenaOn      = false;
    s_sirenaPattern = 0;
    s_sirenaBright  = 200;
    speaker.stop();
    linterna.turnOff();
    drawSirenaState();
    itemLoopCallback = sirenaLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk = []() { sirenaToggle(); drawSirenaState(); };
    cbs.onRight = []() {
        s_sirenaPattern = (s_sirenaPattern + 1) % SIRENA_NUM_PATTERNS;
        s_sirenaT0 = millis(); s_sirenaLastSwap = 0;
        drawSirenaState();
    };
    cbs.onLeft = []() {
        s_sirenaPattern = (s_sirenaPattern + SIRENA_NUM_PATTERNS - 1) % SIRENA_NUM_PATTERNS;
        s_sirenaT0 = millis(); s_sirenaLastSwap = 0;
        drawSirenaState();
    };
    cbs.onUp = []() {
        s_sirenaBright = (uint8_t)min(255, (int)s_sirenaBright + 25);
        drawSirenaState();
    };
    cbs.onDown = []() {
        s_sirenaBright = (uint8_t)max(25, (int)s_sirenaBright - 25);
        drawSirenaState();
    };
    cbs.onMenu = []() { sirenaAllOff(); s_sirenaOn = false; returnToMenu(); };
    buttons.setCallbacks(cbs);
}

// =====================================================
// MIS DISPOSITIVOS — CAM CAR (ESP-NOW)
// =====================================================

static CommandType   s_ccLastCmd       = CMD_STOP;
static unsigned long s_ccLastSendMs    = 0;
static bool          s_ccVideoActive   = false;  // true si la pantalla muestra video
static uint32_t      s_ccFpsCounter    = 0;
static uint32_t      s_ccFpsValue      = 0;
static unsigned long s_ccFpsWindowMs   = 0;

static const char* ccCmdName(CommandType c) {
    switch (c) {
        case CMD_FORWARD:  return "ADELANTE";
        case CMD_BACKWARD: return "ATRAS";
        case CMD_LEFT:     return "IZQUIERDA";
        case CMD_RIGHT:    return "DERECHA";
        default:           return "STOP";
    }
}

static bool ccIsLinked() {
    return camCar.isActive()
        && camCar.getFramesReceived() > 0
        && (millis() - camCar.getLastRxMs() < 2000);
}

static void drawCamCarStatusState() {
    screen.drawCamCarStatus(camCar.isActive(), ccCmdName(s_ccLastCmd), ccIsLinked());
    s_ccVideoActive = false;
}

// Lee direccion mantenida (sin pasar por callbacks de flanco)
static CommandType camCarReadCommand() {
    if (buttons.isUpDown())    return CMD_FORWARD;
    if (buttons.isDownDown())  return CMD_BACKWARD;
    if (buttons.isLeftDown())  return CMD_LEFT;
    if (buttons.isRightDown()) return CMD_RIGHT;
    return CMD_STOP;
}

static void camCarLoop() {
    if (!camCar.isActive()) return;
    unsigned long now = millis();

    // Comando: reenviar si cambia o cada 150ms (refresco anti-perdida de paquete)
    CommandType cmd = camCarReadCommand();
    if (cmd != s_ccLastCmd || now - s_ccLastSendMs >= 150) {
        camCar.sendCommand(cmd);
        s_ccLastCmd    = cmd;
        s_ccLastSendMs = now;
        if (!s_ccVideoActive) drawCamCarStatusState();
    }

    bool linked = ccIsLinked();

    if (camCar.hasFrame()) {
        screen.showJpeg((uint8_t*)camCar.getFrameBuffer(), camCar.getFrameSize(), 160, 120);
        screen.drawCamCarOverlay(ccCmdName(s_ccLastCmd), true, s_ccFpsValue);
        camCar.clearFrame();
        s_ccVideoActive = true;
        s_ccFpsCounter++;
    } else if (s_ccVideoActive && !linked) {
        // Se perdio la señal de video: volver a pantalla de estado
        drawCamCarStatusState();
    }

    if (now - s_ccFpsWindowMs >= 1000) {
        s_ccFpsValue    = s_ccFpsCounter;
        s_ccFpsCounter  = 0;
        s_ccFpsWindowMs = now;
    }
}

void enterCamCar() {
    s_ccLastCmd     = CMD_STOP;
    s_ccLastSendMs  = 0;
    s_ccVideoActive = false;
    s_ccFpsCounter  = 0;
    s_ccFpsValue    = 0;
    s_ccFpsWindowMs = millis();

    speaker.stop();
    camCar.begin();  // ESP-NOW se enciende al entrar; [A] lo apaga/enciende para ahorrar bateria
    drawCamCarStatusState();
    itemLoopCallback = camCarLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk = []() {
        if (camCar.isActive()) camCar.sendCommand(CMD_STOP);
        s_ccLastCmd = CMD_STOP;
        if (!s_ccVideoActive) drawCamCarStatusState();
    };
    cbs.onA = []() {
        if (camCar.isActive()) {
            camCar.sendCommand(CMD_STOP);
            camCar.end();
        } else {
            camCar.begin();
        }
        s_ccLastCmd = CMD_STOP;
        drawCamCarStatusState();
    };
    cbs.onMenu = []() {
        if (camCar.isActive()) {
            camCar.sendCommand(CMD_STOP);
            camCar.end();
        }
        returnToMenu();
    };
    buttons.setCallbacks(cbs);
}

// =====================================================
// MENU NAVIGATION
// =====================================================

void renderMenu() {
    const MenuSection* sec = &sections[currentSection];
    const char* names[16];
    int count = sec->itemCount < 16 ? sec->itemCount : 16;
    for (int i = 0; i < count; i++) {
        names[i] = sec->items[i].name;
    }
    screen.showMenu(sec->title, names, count, currentItem);
}

void itemLoopUpdate() {
    if (itemLoopCallback) itemLoopCallback();
}

// =====================================================
// CALLBACKS - MODO MENU
// =====================================================

static void onMenuLeft()   {
    currentSection = (currentSection + 1) % sectionCount;
    currentItem = 0;
    renderMenu();
}

static void onMenuRight()  {
    currentSection = (currentSection - 1 + sectionCount) % sectionCount;
    currentItem = 0;
    renderMenu();
}

static void onMenuUp()     {
    int count = sections[currentSection].itemCount;
    currentItem = (currentItem - 1 + count) % count;
    renderMenu();
}

static void onMenuDown()   {
    int count = sections[currentSection].itemCount;
    currentItem = (currentItem + 1) % count;
    renderMenu();
}

static void onMenuOk()     {
    itemLoopCallback = nullptr;
    buttons.setCallbacks(getItemCallbacks());
    const MenuItem* item = &sections[currentSection].items[currentItem];
    if (item->onEnter) item->onEnter();
}

static void onMenuMenu()   { renderMenu(); }

ButtonActionCallbacks getMenuCallbacks() {
    ButtonActionCallbacks cbs;
    cbs.onMenu  = onMenuMenu;
    cbs.onLeft  = onMenuLeft;
    cbs.onRight = onMenuRight;
    cbs.onUp    = onMenuUp;
    cbs.onDown  = onMenuDown;
    cbs.onOk    = onMenuOk;
    return cbs;
}

// =====================================================
// CALLBACKS - MODO ITEM (generico)
// =====================================================

static void onItemOk() {
    const MenuItem* item = &sections[currentSection].items[currentItem];
    if (item->onEnter) item->onEnter();
}

void returnToMenu() {
    itemLoopCallback = nullptr;
    speaker.stop();
    buttons.setCallbacks(getMenuCallbacks());
    renderMenu();
}

ButtonActionCallbacks getItemCallbacks() {
    ButtonActionCallbacks cbs;
    cbs.onOk   = onItemOk;
    cbs.onMenu = returnToMenu;
    return cbs;
}
