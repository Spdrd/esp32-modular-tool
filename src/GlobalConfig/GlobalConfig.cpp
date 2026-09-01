#include "GlobalConfig/GlobalConfig.h"
#include <math.h>

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

static JoystickPinConfig joystickConfig = {
    .xPin            = JOY_X_PIN,
    .yPin            = JOY_Y_PIN,
    .swPin           = JOY_SW_PIN,
    .enablePin       = JOY_ENABLE_PIN,
    .enableActiveLow = JOY_ENABLE_ACTIVE_LOW,
    .invertX         = JOY_INVERT_X,
    .invertY         = JOY_INVERT_Y
};

ButtonManager buttons(buttonConfig);
JoystickManager joystick(joystickConfig);
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

static const uint8_t ledStripMac[6] = LEDSTRIP_MAC;
EspNowLedManager ledStrip(ledStripMac);

static BluetoothConfig btConfig = {
    .deviceName   = "ESP32 Tool",
    .manufacturer = "J. Diego",
    .vid          = 0x05AC,   // se anuncia como teclado generico
    .pid          = 0x820A,
    .version      = 0x0100
};
BluetoothManager bt(btConfig);

PongGame        pong;
BreakoutGame    breakout;
FlappyGame      flappy;
InvadersGame    invaders;
MinesweeperGame minesweeper;
DoomGame        doom;

// =====================================================
// MENU DATA
// =====================================================

static const MenuItem infoItems[] = {
    {"Version 1.0", enterVersion},
    {"Creditos",    enterCredits},
};

static const MenuItem gamesItems[] = {
    {"Snake",        enterSnake},
    {"Simon",        enterSimon},
    {"Tetris",       enterTetris},
    {"2048",         enterGame2048},
    {"Pong",         enterPong},
    {"Breakout",     enterBreakout},
    {"Flappy Bird",  enterFlappy},
    {"Invaders",     enterInvaders},
    {"Buscaminas",   enterMinesweeper},
    {"DOOM",         enterDoom},
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
    {"Joystick",    enterTestJoystick},
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
    {"Cam Car",  enterCamCar},
    {"Tira LED", enterLedStrip},
};

static const MenuItem bluetoothItems[] = {
    {"Musica",            enterMusicControl},
    {"Mouse",             enterBtMouse},
    {"Teclado WASD",      enterTecladoWasd},
    {"Teclado Minecraft", enterTecladoMinecraft}
};

static const MenuSection menuSections[] = {
    {"Info",       infoItems,      2},
    {"Bluetooth",  bluetoothItems, 4},
    {"Mis Dispositivos", devicesItems, 2},
    {"Herramientas",toolsItems,    9},
    {"Juegos",     gamesItems,    10},
    {"Musica",     musicItems,     8}
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

// El LED RGB acompaña a la pantalla con los mismos colores que usa drawSimon()
// (0=UP verde, 1=DOWN rojo, 2=LEFT azul, 3=RIGHT amarillo).
static const uint8_t SIMON_LED_RGB[4][3] = {
    {   0, 255,   0 },  // UP
    { 255,   0,   0 },  // DOWN
    {   0,   0, 255 },  // LEFT
    { 255, 255,   0 },  // RIGHT
};
static const uint8_t SIMON_LED_BRIGHT = 160;

static void simonShowLed() {
    int hl = simon.getHighlight();

    // Al perder, el boton equivocado parpadea en pantalla: el LED sigue el
    // mismo ritmo apagandose en los frames en que el boton esta apagado.
    bool off = (hl < 0) ||
               (simon.getState() == SimonGame::LOSE && !simon.isFlashOn());

    if (off) {
        linterna.turnOff();
        return;
    }

    linterna.showColor(SIMON_LED_RGB[hl][0],
                       SIMON_LED_RGB[hl][1],
                       SIMON_LED_RGB[hl][2],
                       SIMON_LED_BRIGHT);
}

// Vuelca el estado del juego a las dos salidas: pantalla y LED.
static void drawSimonState() {
    screen.drawSimon(
        simon.getHighlight(),
        simon.getScore(),
        (int)simon.getState(),
        simon.isFlashOn()
    );
    simonShowLed();
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
    cbs.onMenu  = []() { linterna.turnOff(); returnToMenu(); };
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

// DAS para tetris
static unsigned long s_tetDasStart  = 0;
static unsigned long s_tetDasRepeat = 0;
static int  s_tetDasDir   = 0; // -1=izq, 1=der, 0=nada
static unsigned long s_tetDropRepeat = 0;
static const unsigned long TET_DAS_DELAY  = 220;
static const unsigned long TET_DAS_PERIOD = 60;
static const unsigned long TET_DROP_PERIOD = 50;

static void tetrisLoop() {
    bool redraw = tetris.update();
    if (tetris.isGameOver()) { if (redraw) drawTetrisState(); return; }

    unsigned long now = millis();

    // Soft drop continuo mientras down está pulsado
    if (inputDown() && now - s_tetDropRepeat >= TET_DROP_PERIOD) {
        tetris.softDrop();
        s_tetDropRepeat = now;
        redraw = true;
    }

    // DAS lateral
    int dir = 0;
    if      (inputLeft())  dir = -1;
    else if (inputRight()) dir =  1;

    if (dir != 0) {
        if (dir != s_tetDasDir) {
            (dir < 0) ? tetris.moveLeft() : tetris.moveRight();
            redraw = true;
            s_tetDasStart  = now;
            s_tetDasRepeat = now;
            s_tetDasDir    = dir;
        } else if (now - s_tetDasStart >= TET_DAS_DELAY &&
                   now - s_tetDasRepeat >= TET_DAS_PERIOD) {
            (dir < 0) ? tetris.moveLeft() : tetris.moveRight();
            redraw = true;
            s_tetDasRepeat = now;
        }
    } else {
        s_tetDasDir = 0;
    }

    if (redraw) drawTetrisState();
}

void enterTetris() {
    tetris.reset();
    s_tetDasDir = 0; s_tetDasStart = 0; s_tetDasRepeat = 0; s_tetDropRepeat = 0;
    drawTetrisState();
    itemLoopCallback = tetrisLoop;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { tetris.hardDrop();   drawTetrisState(); };
    // left/right/down manejados por held en tetrisLoop; no se asignan callbacks
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

    bool okDown = inputOk();
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
        bool okDown = inputOk();
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

// =====================================================
// PONG
// =====================================================

static void drawPongState() {
    screen.drawPong(pong.getBallX(), pong.getBallY(),
                    pong.getPlayerY(), pong.getAiY(),
                    pong.getPlayerScore(), pong.getAiScore(),
                    pong.isGameOver(), pong.playerWon());
}

static void pongLoop() {
    bool moved = false;
    if (inputUp())   { pong.moveUp();   moved = true; }
    if (inputDown()) { pong.moveDown(); moved = true; }
    bool updated = pong.update();
    if (updated || moved) drawPongState();
}

void enterPong() {
    pong.reset();
    drawPongState();
    itemLoopCallback = pongLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk   = []() { if (pong.isGameOver()) { pong.reset(); drawPongState(); } };
    cbs.onMenu = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// BREAKOUT
// =====================================================

static void drawBreakoutState() {
    screen.drawBreakout(
        reinterpret_cast<const bool(*)[7]>(breakout.getBricks()),
        BreakoutGame::ROWS, BreakoutGame::COLS,
        BreakoutGame::BRICK_W, BreakoutGame::BRICK_H,
        breakout.getPaddleX(), breakout.getBallX(), breakout.getBallY(),
        breakout.getScore(), breakout.getLives(),
        breakout.isLaunched(), breakout.isGameOver(), breakout.isWon());
}

static void breakoutLoop() {
    bool moved = false;
    if (inputLeft())  { breakout.moveLeft();  moved = true; }
    if (inputRight()) { breakout.moveRight(); moved = true; }
    bool updated = breakout.update();
    if (updated || moved) drawBreakoutState();
}

void enterBreakout() {
    breakout.reset();
    drawBreakoutState();
    itemLoopCallback = breakoutLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk    = []() {
        if (breakout.isGameOver() || breakout.isWon()) { breakout.reset(); drawBreakoutState(); }
        else breakout.launch();
    };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// FLAPPY BIRD
// =====================================================

static void drawFlappyState() {
    screen.drawFlappy(flappy.getBirdY(), flappy.getPipeX(), flappy.getGapY(),
                      flappy.getScore(), flappy.isGameOver(), flappy.isStarted());
}

static void flappyLoop() {
    if (flappy.update()) drawFlappyState();
}

void enterFlappy() {
    flappy.reset();
    drawFlappyState();
    itemLoopCallback = flappyLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk  = []() {
        if (flappy.isGameOver()) { flappy.reset(); drawFlappyState(); }
        else flappy.flap();
    };
    cbs.onA    = []() {
        if (flappy.isGameOver()) { flappy.reset(); drawFlappyState(); }
        else flappy.flap();
    };
    cbs.onMenu = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// SPACE INVADERS
// =====================================================

static void drawInvadersState() {
    screen.drawInvaders(
        reinterpret_cast<const bool(*)[6]>(invaders.getGrid()),
        InvadersGame::ROWS, InvadersGame::COLS,
        invaders.getInvOfsX(), invaders.getInvOfsY(), InvadersGame::INV_SPACING,
        invaders.getShipX(), invaders.getScore(),
        invaders.getBullet().x, invaders.getBullet().y, invaders.getBullet().active,
        invaders.getInvBullet().x, invaders.getInvBullet().y, invaders.getInvBullet().active,
        invaders.isGameOver(), invaders.isWon());
}

static unsigned long s_invMoveMs = 0;

static void invadersLoop() {
    bool moved = false;
    unsigned long now = millis();
    if (now - s_invMoveMs >= 80) {
        if (inputLeft())  { invaders.moveLeft();  moved = true; s_invMoveMs = now; }
        if (inputRight()) { invaders.moveRight(); moved = true; s_invMoveMs = now; }
    }
    bool updated = invaders.update();
    if (updated || moved) drawInvadersState();
}

void enterInvaders() {
    invaders.reset();
    s_invMoveMs = 0;
    drawInvadersState();
    itemLoopCallback = invadersLoop;

    ButtonActionCallbacks cbs;
    // left/right son held en invadersLoop; solo disparo en callbacks
    cbs.onOk    = []() {
        if (invaders.isGameOver() || invaders.isWon()) { invaders.reset(); drawInvadersState(); }
        else invaders.shoot();
    };
    cbs.onA     = []() {
        if (!invaders.isGameOver() && !invaders.isWon()) invaders.shoot();
    };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// MINESWEEPER
// =====================================================

static void drawMinesweeperState() {
    screen.drawMinesweeper(
        reinterpret_cast<const bool(*)[9]>(minesweeper.getRevealed()),
        reinterpret_cast<const bool(*)[9]>(minesweeper.getFlagged()),
        reinterpret_cast<const bool(*)[9]>(minesweeper.getMines()),
        reinterpret_cast<const int (*)[9]>(minesweeper.getAdjs()),
        MinesweeperGame::ROWS, MinesweeperGame::COLS,
        minesweeper.getCursorX(), minesweeper.getCursorY(),
        minesweeper.getFlagsLeft(), (int)minesweeper.getState());
}

// DAS para minesweeper: delay inicial antes de repetir, luego periodo rapido
static unsigned long s_msDasStart  = 0; // cuando se empezo a mantener
static unsigned long s_msDasRepeat = 0; // ultimo movimiento repetido
static int s_msDasDx = 0, s_msDaDy = 0; // direccion activa
static const unsigned long MS_DAS_DELAY  = 380; // ms antes de empezar a repetir
static const unsigned long MS_DAS_PERIOD = 130; // ms entre repeticiones

static void minesweeperLoop() {
    if (minesweeper.getState() != MinesweeperGame::PLAYING) return;

    int dx = 0, dy = 0;
    if      (inputLeft())  dx = -1;
    else if (inputRight()) dx =  1;
    else if (inputUp())    dy = -1;
    else if (inputDown())  dy =  1;

    unsigned long now = millis();

    if (dx != 0 || dy != 0) {
        bool dirChanged = (dx != s_msDasDx || dy != s_msDaDy);
        if (dirChanged) {
            // Primera pulsacion: mover inmediatamente y arrancar DAS
            minesweeper.moveCursor(dx, dy);
            drawMinesweeperState();
            s_msDasStart  = now;
            s_msDasRepeat = now;
            s_msDasDx = dx; s_msDaDy = dy;
        } else if (now - s_msDasStart >= MS_DAS_DELAY &&
                   now - s_msDasRepeat >= MS_DAS_PERIOD) {
            minesweeper.moveCursor(dx, dy);
            drawMinesweeperState();
            s_msDasRepeat = now;
        }
    } else {
        s_msDasDx = 0; s_msDaDy = 0;
    }
}

void enterMinesweeper() {
    minesweeper.reset();
    s_msDasDx = 0; s_msDaDy = 0; s_msDasStart = 0; s_msDasRepeat = 0;
    drawMinesweeperState();
    itemLoopCallback = minesweeperLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk    = []() {
        if (minesweeper.getState() != MinesweeperGame::PLAYING) {
            minesweeper.reset(); drawMinesweeperState();
        } else {
            minesweeper.reveal(); drawMinesweeperState();
        }
    };
    cbs.onA     = []() {
        if (minesweeper.getState() == MinesweeperGame::PLAYING) {
            minesweeper.toggleFlag(); drawMinesweeperState();
        }
    };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// DOOM
// =====================================================

static void drawDoomState() {
    screen.drawDoom(doom.getPlayerX(), doom.getPlayerY(), doom.getAngle(),
                    doom.getHealth(), doom.getAmmo(), doom.getKills(),
                    doom.isShooting(), doom.isGameOver(), doom.isAllDead(),
                    doom.getEnemies(), DoomGame::MAX_ENEMIES);
}

static void doomLoop() {
    bool redraw = doom.update();

    // Held-button movement (continuo)
    bool moved = false;
    if (inputUp())    { doom.moveForward();  moved = true; }
    if (inputDown())  { doom.moveBackward(); moved = true; }
    if (inputLeft())  { doom.turnLeft();     moved = true; }
    if (inputRight()) { doom.turnRight();    moved = true; }

    if (redraw || moved) drawDoomState();
}

void enterDoom() {
    doom.reset();
    drawDoomState();
    itemLoopCallback = doomLoop;

    ButtonActionCallbacks cbs;
    cbs.onA    = []() { doom.shoot(); drawDoomState(); };
    cbs.onOk   = []() {
        if (doom.isGameOver() || doom.isAllDead()) { doom.reset(); drawDoomState(); }
        else { doom.shoot(); drawDoomState(); }
    };
    cbs.onMenu = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// SNAKE
// =====================================================

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
    if (inputUp())    return CMD_FORWARD;
    if (inputDown())  return CMD_BACKWARD;
    if (inputLeft())  return CMD_LEFT;
    if (inputRight()) return CMD_RIGHT;
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
        bool valid = screen.showJpeg((uint8_t*)camCar.getFrameBuffer(), camCar.getFrameSize(), 160, 120);
        if (valid) {
            screen.drawCamCarOverlay(ccCmdName(s_ccLastCmd), true, s_ccFpsValue);
            s_ccVideoActive = true;
            s_ccFpsCounter++;
        } else if (s_ccVideoActive) {
            // Frame negro descartado: re-dibujar overlay sobre el ultimo frame valido
            screen.drawCamCarOverlay(ccCmdName(s_ccLastCmd), true, s_ccFpsValue);
        }
        camCar.clearFrame();
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
// TEST JOYSTICK
// =====================================================

static void drawJoystickState() {
    screen.drawJoystick(joystick.getRawX(), joystick.getRawY(),
                        joystick.getCenterX(), joystick.getCenterY(),
                        joystick.getX(), joystick.getY(),
                        joystick.getMagnitude(), joystick.getAngleDeg(),
                        joystick.isUpDown(), joystick.isDownDown(),
                        joystick.isLeftDown(), joystick.isRightDown(),
                        joystick.isButtonDown(),
                        joyGestureName(joystick.getPendingGesture()));
}

static void joystickTestLoop() {
    // Redibujar a ritmo fijo: el SPI de la pantalla es lento y el stick da
    // valores nuevos en cada vuelta del loop.
    static unsigned long lastMs = 0;
    unsigned long now = millis();
    if (now - lastMs < 80) return;
    lastMs = now;
    drawJoystickState();
}

void enterTestJoystick() {
    speaker.stop();
    drawJoystickState();
    itemLoopCallback = joystickTestLoop;

    ButtonActionCallbacks cbs;
    // Sin callbacks de direccion: aqui interesa ver el stick crudo, no navegar.
    // Recalibrar va en B y no en OK para que sea deliberado: un click corto
    // accidental dejaria el centro mal medido.
    cbs.onB    = []() {
        // El gesto se resuelve al soltar, pero la perilla tarda en volver al
        // centro. Sin esta espera se calibraria sobre un stick aun desviado.
        delay(250);
        joystick.calibrateCenter();
        Serial.println("[JOY] centro recalibrado");
        drawJoystickState();
    };
    cbs.onMenu = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// TIRA LED (ESP-NOW)
// =====================================================

// UP/DOWN elige parametro, LEFT/RIGHT lo ajusta (con repeticion al mantener).
// Cada cambio se envia solo, limitado a un paquete cada 60ms para no saturar
// la radio mientras se mantiene pulsado.

enum { LP_EFFECT = 0, LP_BRIGHT, LP_R, LP_G, LP_B, LP_BPS, LP_COUNT };

static int       s_lsParam = LP_EFFECT;
// Estado inicial: blanco solido a brillo maximo. Al entrar se reenvia, asi la
// tira arranca siempre en un punto conocido sin importar como quedo antes.
// Campos: { effect, brightness, r, g, b, bps }
static const LedPacket LS_DEFAULT_PKT = { LED_FX_SOLID, 255, 255, 255, 255, 20 };

static LedPacket s_lsPkt   = LS_DEFAULT_PKT;

static uint8_t   s_lsBrightBackup = 255;   // para el apagado rapido con [B]

static unsigned long s_lsSendMs   = 0;
static unsigned long s_lsAdjStart = 0;
static unsigned long s_lsAdjRep   = 0;
static int           s_lsAdjDir   = 0;

static const unsigned long LS_SEND_PERIOD_MS = 60;
static const unsigned long LS_ADJ_DELAY_MS   = 350;
static const unsigned long LS_ADJ_PERIOD_MS  = 70;

static void drawLedStripState() {
    screen.drawLedStrip(s_lsParam, s_lsPkt.effect, ledEffectName(s_lsPkt.effect),
                        s_lsPkt.brightness, s_lsPkt.r, s_lsPkt.g, s_lsPkt.b,
                        s_lsPkt.bps, ledStrip.isActive(), ledStrip.lastSendOk(),
                        ledStrip.isBroadcast(),
                        ledStrip.getSentCount(), ledStrip.getFailCount());
}

// Envia respetando el limite de ritmo. force ignora el limite ([OK]).
static void ledStripSend(bool force) {
    unsigned long now = millis();
    if (!force && now - s_lsSendMs < LS_SEND_PERIOD_MS) return;
    s_lsSendMs = now;
    ledStrip.send(s_lsPkt);
}

// Aplica un paso al parametro activo, respetando el rango de cada campo.
static void ledStripAdjust(int dir) {
    switch (s_lsParam) {
        case LP_EFFECT: {
            int v = (int)s_lsPkt.effect + dir;
            if (v < 0) v = LED_FX_COUNT - 1;
            if (v >= LED_FX_COUNT) v = 0;
            s_lsPkt.effect = (uint8_t)v;
            break;
        }
        case LP_BRIGHT: s_lsPkt.brightness = constrain((int)s_lsPkt.brightness + dir * 5, 0, 255); break;
        case LP_R:      s_lsPkt.r          = constrain((int)s_lsPkt.r          + dir * 5, 0, 255); break;
        case LP_G:      s_lsPkt.g          = constrain((int)s_lsPkt.g          + dir * 5, 0, 255); break;
        case LP_B:      s_lsPkt.b          = constrain((int)s_lsPkt.b          + dir * 5, 0, 255); break;
        // El receptor recorta bps a [1,60]; se limita aqui para que la
        // pantalla muestre lo mismo que va a aplicar la tira.
        case LP_BPS:    s_lsPkt.bps        = constrain((int)s_lsPkt.bps        + dir,     1,  60); break;
    }
}

static void ledStripLoop() {
    unsigned long now = millis();

    // LEFT/RIGHT sostenidos ajustan el valor del parametro activo
    int dir = 0;
    if      (inputRight()) dir =  1;
    else if (inputLeft())  dir = -1;

    if (dir != 0) {
        bool first = (dir != s_lsAdjDir);
        if (first || (now - s_lsAdjStart >= LS_ADJ_DELAY_MS &&
                      now - s_lsAdjRep   >= LS_ADJ_PERIOD_MS)) {
            ledStripAdjust(dir);
            ledStripSend(false);
            drawLedStripState();
            s_lsAdjRep = now;
            if (first) { s_lsAdjDir = dir; s_lsAdjStart = now; }
        }
    } else {
        s_lsAdjDir = 0;
    }
}

void enterLedStrip() {
    speaker.stop();
    ledStrip.begin();

    // Se vuelve al blanco a tope en cada entrada: el ESP32 no sabe que esta
    // mostrando la tira (no hay ACK de aplicacion), asi que partir siempre del
    // mismo estado es lo unico que garantiza que pantalla y tira coinciden.
    s_lsPkt           = LS_DEFAULT_PKT;
    s_lsBrightBackup  = LS_DEFAULT_PKT.brightness;

    s_lsParam   = LP_EFFECT;
    s_lsAdjDir  = 0;
    s_lsSendMs  = 0;
    ledStripSend(true);    // estado inicial, para que la tira arranque igual
    drawLedStripState();

    itemLoopCallback = ledStripLoop;

    ButtonActionCallbacks cbs;
    // LEFT/RIGHT se leen sostenidos en ledStripLoop para ajustar el valor.
    // UP/DOWN se mueven entre parametros (por callback, no sostenido).
    cbs.onUp    = []() {
        s_lsParam = (s_lsParam - 1 + LP_COUNT) % LP_COUNT;
        drawLedStripState();
    };
    cbs.onDown  = []() {
        s_lsParam = (s_lsParam + 1) % LP_COUNT;
        drawLedStripState();
    };
    cbs.onA     = []() {
        s_lsPkt.effect = (s_lsPkt.effect + 1) % LED_FX_COUNT;
        ledStripSend(true);
        drawLedStripState();
    };
    cbs.onB     = []() {
        // Apagado rapido: brillo 0 y de vuelta. El receptor no tiene comando
        // de apagado, asi que apagar es poner el brillo a cero.
        if (s_lsPkt.brightness > 0) {
            s_lsBrightBackup   = s_lsPkt.brightness;
            s_lsPkt.brightness = 0;
        } else {
             s_lsPkt.brightness = s_lsBrightBackup > 0 ? s_lsBrightBackup : 128;
        }
        ledStripSend(true);
        drawLedStripState();
    };
    cbs.onOk    = []() { ledStripSend(true); drawLedStripState(); };
    cbs.onMenu  = []() { ledStrip.end(); returnToMenu(); };
    buttons.setCallbacks(cbs);
}

// =====================================================
// BLUETOOTH - MOUSE
// =====================================================

// Stick mueve el puntero, boton del stick = autoscroll (boton central),
// A/B = click izquierdo/derecho, UP/DOWN = rueda, LEFT/RIGHT = pestanas.

static bool          s_msLeft      = false;
static bool          s_msRight     = false;
static bool          s_msMiddle    = false;
static bool          s_msConnected = false;
static const char*   s_msAction    = "";
static unsigned long s_msActionMs  = 0;
static unsigned long s_msMoveMs    = 0;

static const unsigned long MS_MOVE_PERIOD_MS   = 20;    // 50 reportes/s
static const unsigned long MS_ACTION_HOLD_MS   = 900;
static const float         MS_MAX_SPEED        = 14.0f; // px por reporte

static void drawBtMouseState() {
    screen.drawBtMouse(bt.isActive(), bt.isConnected(), bt.getDeviceName(),
                       joystick.getX(), joystick.getY(),
                       s_msLeft, s_msRight, s_msMiddle, s_msAction);
}

static void msFlash(const char* label) {
    s_msAction   = label;
    s_msActionMs = millis();
}

static uint8_t msButtonMask() {
    uint8_t m = 0;
    if (s_msLeft)   m |= HID_MOUSE_LEFT;
    if (s_msRight)  m |= HID_MOUSE_RIGHT;
    if (s_msMiddle) m |= HID_MOUSE_MIDDLE;
    return m;
}

// Curva de aceleracion: el desplazamiento crece con el cuadrado de la
// desviacion, para tener precision fina cerca del centro y velocidad util
// en los extremos.
static int msAxisDelta(float v) {
    if (v == 0.0f) return 0;
    float mag = fabsf(v);
    float d   = mag * mag * MS_MAX_SPEED;
    int   out = (int)(d + 0.5f);
    if (out == 0) out = 1;              // que un empuje minimo siempre mueva
    return (v < 0) ? -out : out;
}

static void btMouseLoop() {
    unsigned long now = millis();

    // --- Boton central sostenido (autoscroll) ---
    // Los gestos estan apagados en esta herramienta, asi que el boton del
    // stick se lee crudo.
    bool middleNow = joystick.isButtonDown();
    if (middleNow != s_msMiddle) {
        s_msMiddle = middleNow;
        bt.mouseSetButtons(msButtonMask());
        msFlash(s_msMiddle ? "AUTOSCROLL" : "");
        drawBtMouseState();
    }

    // --- Movimiento del puntero ---
    if (now - s_msMoveMs >= MS_MOVE_PERIOD_MS) {
        s_msMoveMs = now;
        int dx = msAxisDelta(joystick.getX());
        // El eje Y de la pantalla crece hacia abajo, al reves que el stick
        int dy = -msAxisDelta(joystick.getY());
        if (dx != 0 || dy != 0) bt.mouseMove(dx, dy);
    }

    // Limpiar la etiqueta pasado su tiempo
    if (s_msActionMs != 0 && now - s_msActionMs >= MS_ACTION_HOLD_MS) {
        s_msAction   = "";
        s_msActionMs = 0;
        drawBtMouseState();
    }

    // Solo se redibuja ante cambios reales. Un refresco periodico daria un
    // indicador del stick mas vivo, pero cada repintado bloquea el SPI decenas
    // de ms y eso se nota como tirones en el puntero, que va a 50 reportes/s.
    if (bt.isConnected() != s_msConnected) {
        s_msConnected = bt.isConnected();
        drawBtMouseState();
    }
}

void enterBtMouse() {
    speaker.stop();
    bt.begin();

    // Aqui la cruz y el stick hacen cosas DISTINTAS, asi que se corta el
    // espejo: el stick solo alimenta el puntero por su interfaz analogica, y
    // las direcciones (scroll y pestanas) llegan solo de la cruz fisica. Con
    // el espejo puesto, empujar el stick arriba movia el cursor y ademas
    // disparaba onUp, o sea scroll simultaneo.
    joystick.mirror(nullptr);

    // El boton del stick es el autoscroll: se mantiene pulsado con sentido
    // propio, asi que su duracion no puede significar ademas A, B o MENU.
    joystick.setGesturesEnabled(false);

    s_msLeft = s_msRight = s_msMiddle = false;
    s_msAction    = "";
    s_msActionMs  = 0;
    s_msMoveMs    = 0;
    s_msConnected = bt.isConnected();
    drawBtMouseState();

    itemLoopCallback = btMouseLoop;

    ButtonActionCallbacks cbs;
    cbs.onA = []() {
        s_msLeft = !s_msLeft;   // toggle: permite arrastrar sin mantener
        bt.mouseSetButtons(msButtonMask());
        msFlash(s_msLeft ? "IZQ ABAJO" : "IZQ CLICK");
        drawBtMouseState();
    };
    cbs.onB = []() {
        s_msRight = !s_msRight;
        bt.mouseSetButtons(msButtonMask());
        msFlash(s_msRight ? "DER ABAJO" : "DER CLICK");
        drawBtMouseState();
    };
    cbs.onUp    = []() { bt.mouseScroll( 1); msFlash("SCROLL +"); drawBtMouseState(); };
    cbs.onDown  = []() { bt.mouseScroll(-1); msFlash("SCROLL -"); drawBtMouseState(); };
    // Cambiar de pestana es un atajo de teclado, no una accion de mouse
    cbs.onRight = []() {
        bt.keyTap(HID_KEY_TAB, HID_MOD_LCTRL);
        msFlash("PESTANA >");
        drawBtMouseState();
    };
    cbs.onLeft  = []() {
        bt.keyTap(HID_KEY_TAB, HID_MOD_LCTRL | HID_MOD_LSHIFT);
        msFlash("< PESTANA");
        drawBtMouseState();
    };
    cbs.onOk    = []() {
        // Soltar todo, por si quedo algun boton enganchado
        s_msLeft = s_msRight = false;
        bt.mouseSetButtons(msButtonMask());
        msFlash("SOLTAR");
        drawBtMouseState();
    };
    cbs.onMenu  = []() {
        bt.mouseSetButtons(0);   // no dejar botones pulsados en el host
        bt.end();
        returnToMenu();          // reactiva los gestos del stick
    };
    buttons.setCallbacks(cbs);
}

// =====================================================
// BLUETOOTH - TECLADO WASD
// =====================================================

// Mapa de botones fisicos -> teclas HID:
//   UP=W  LEFT=A  DOWN=S  RIGHT=D  B=ESC  OK=ENTER  A=SPACE  MENU=salir
// Las teclas se mantienen pulsadas mientras el boton este presionado, para
// que sirvan de verdad en juegos (no son taps sueltos).

static uint8_t s_btKbMask      = 0;
static bool    s_btKbConnected = false;

static void btWASDKeyboardLoop() {
    uint8_t keys[6];
    uint8_t n    = 0;
    uint8_t mask = 0;

    // mask refleja siempre el estado fisico; el reporte HID solo admite 6
    if (inputUp())    { mask |= 0x01; if (n < 6) keys[n++] = HID_KEY_W;     }
    if (inputLeft())  { mask |= 0x02; if (n < 6) keys[n++] = HID_KEY_A;     }
    if (inputDown())  { mask |= 0x04; if (n < 6) keys[n++] = HID_KEY_S;     }
    if (inputRight()) { mask |= 0x08; if (n < 6) keys[n++] = HID_KEY_D;     }
    if (buttons.isBDown())     { mask |= 0x10; if (n < 6) keys[n++] = HID_KEY_ESC;   }
    if (inputOk())    { mask |= 0x20; if (n < 6) keys[n++] = HID_KEY_SPACE; }
    if (buttons.isADown())     { mask |= 0x40; if (n < 6) keys[n++] = HID_KEY_ENTER; }

    // keyboardReport ya descarta envios identicos al anterior
    bt.keyboardReport(HID_MOD_NONE, keys, n);

    // Redibujar solo cuando cambia algo: el SPI de la pantalla es lento
    bool conn = bt.isConnected();
    if (mask != s_btKbMask || conn != s_btKbConnected) {
        s_btKbMask      = mask;
        s_btKbConnected = conn;
        screen.drawBtKeyboard(bt.isActive(), conn, bt.getDeviceName(), mask);
    }
}

void enterTecladoWasd() {
    speaker.stop();
    bt.begin();   // BLE se enciende al entrar y se apaga al salir con [MENU]

    s_btKbMask      = 0;
    s_btKbConnected = bt.isConnected();
    screen.drawBtKeyboard(bt.isActive(), s_btKbConnected, bt.getDeviceName(), 0);

    itemLoopCallback = btWASDKeyboardLoop;

    ButtonActionCallbacks cbs;
    // Todas las teclas se leen por estado sostenido en btKeyboardLoop.
    // MENU es el unico que no se envia: sale y apaga la radio.
    cbs.onMenu = []() {
        bt.keyReleaseAll();
        bt.end();
        returnToMenu();
    };
    buttons.setCallbacks(cbs);
}

static void btMinecraftKeyboardLoop() {
    uint8_t keys[6];
    uint8_t n    = 0;
    uint8_t mask = 0;

    // mask refleja siempre el estado fisico; el reporte HID solo admite 6
    if (inputUp())    { mask |= 0x01; if (n < 6) keys[n++] = HID_KEY_W;     }
    if (inputLeft())  { mask |= 0x02; if (n < 6) keys[n++] = HID_KEY_A;     }
    if (inputDown())  { mask |= 0x04; if (n < 6) keys[n++] = HID_KEY_S;     }
    if (inputRight()) { mask |= 0x08; if (n < 6) keys[n++] = HID_KEY_D;     }
    if (buttons.isBDown())     { mask |= 0x10; if (n < 6) keys[n++] = HID_KEY_ESC;   }
    if (inputOk())    { mask |= 0x20; if (n < 6) keys[n++] = HID_KEY_SPACE; }
    if (buttons.isADown())     { mask |= 0x40; if (n < 6) keys[n++] = HID_KEY_E; }

    // keyboardReport ya descarta envios identicos al anterior
    bt.keyboardReport(HID_MOD_NONE, keys, n);

    // Redibujar solo cuando cambia algo: el SPI de la pantalla es lento
    bool conn = bt.isConnected();
    if (mask != s_btKbMask || conn != s_btKbConnected) {
        s_btKbMask      = mask;
        s_btKbConnected = conn;
        screen.drawBtKeyboard(bt.isActive(), conn, bt.getDeviceName(), mask);
    }
}

void enterTecladoMinecraft() {
    speaker.stop();
    bt.begin();   // BLE se enciende al entrar y se apaga al salir con [MENU]

    s_btKbMask      = 0;
    s_btKbConnected = bt.isConnected();
    screen.drawBtKeyboard(bt.isActive(), s_btKbConnected, bt.getDeviceName(), 0);

    itemLoopCallback = btMinecraftKeyboardLoop;

    ButtonActionCallbacks cbs;
    // Todas las teclas se leen por estado sostenido en btKeyboardLoop.
    // MENU es el unico que no se envia: sale y apaga la radio.
    cbs.onMenu = []() {
        bt.keyReleaseAll();
        bt.end();
        returnToMenu();
    };
    buttons.setCallbacks(cbs);
}

// =====================================================
// BLUETOOTH - CONTROL DE MUSICA
// =====================================================

// UP/DOWN=volumen (con repeticion al mantener)  LEFT/RIGHT=pista
// OK=play/pause  MENU=salir
//
// s_mcPlaying es un estado LOCAL: el host no nos informa que esta sonando,
// solo recibe ordenes. Si la reproduccion se controla desde otro lado, este
// icono se puede desincronizar.
static bool          s_mcPlaying    = false;
static bool          s_mcConnected  = false;
static const char*   s_mcAction     = "";
static unsigned long s_mcActionMs   = 0;
static int           s_mcVolDir     = 0;
static unsigned long s_mcVolStart   = 0;
static unsigned long s_mcVolRepeat  = 0;

static const unsigned long MC_ACTION_HOLD_MS = 1000;  // cuanto dura la etiqueta
static const unsigned long MC_VOL_DELAY_MS   = 400;   // antes de repetir volumen
static const unsigned long MC_VOL_PERIOD_MS  = 180;   // entre repeticiones

static void drawBtMusicState() {
    screen.drawMusicControl(bt.isActive(), bt.isConnected(), bt.getDeviceName(),
                            s_mcPlaying, s_mcAction);
}

static void mcFlash(const char* label) {
    s_mcAction   = label;
    s_mcActionMs = millis();
}

static void btMusicLoop() {
    unsigned long now = millis();
    bool redraw = false;

    // Volumen: primera pulsacion inmediata, luego repeticion al mantener
    int dir = 0;
    if      (inputUp())   dir =  1;
    else if (inputDown()) dir = -1;

    if (dir != 0) {
        bool firstPress = (dir != s_mcVolDir);
        if (firstPress || (now - s_mcVolStart >= MC_VOL_DELAY_MS &&
                           now - s_mcVolRepeat >= MC_VOL_PERIOD_MS)) {
            bt.consumerTap(dir > 0 ? HID_CC_VOL_UP : HID_CC_VOL_DOWN);
            mcFlash(dir > 0 ? "VOL +" : "VOL -");
            redraw = true;
            s_mcVolRepeat = now;
            if (firstPress) { s_mcVolDir = dir; s_mcVolStart = now; }
        }
    } else {
        s_mcVolDir = 0;
    }

    // Borrar la etiqueta pasado su tiempo
    if (s_mcActionMs != 0 && now - s_mcActionMs >= MC_ACTION_HOLD_MS) {
        s_mcAction   = "";
        s_mcActionMs = 0;
        redraw = true;
    }

    bool conn = bt.isConnected();
    if (conn != s_mcConnected) {
        s_mcConnected = conn;
        redraw = true;
    }

    if (redraw) drawBtMusicState();
}

void enterMusicControl() {
    speaker.stop();
    bt.begin();

    s_mcPlaying   = false;
    s_mcAction    = "";
    s_mcActionMs  = 0;
    s_mcVolDir    = 0;
    s_mcConnected = bt.isConnected();
    drawBtMusicState();

    itemLoopCallback = btMusicLoop;

    ButtonActionCallbacks cbs;
    // UP/DOWN se leen sostenidos en btMusicLoop para poder repetir volumen
    cbs.onLeft  = []() { bt.consumerTap(HID_CC_PREV); mcFlash("<< PREV"); drawBtMusicState(); };
    cbs.onRight = []() { bt.consumerTap(HID_CC_NEXT); mcFlash("NEXT >>"); drawBtMusicState(); };
    cbs.onA    = []() {
        bt.consumerTap(HID_CC_PLAY_PAUSE);
        s_mcPlaying = !s_mcPlaying;
        mcFlash(s_mcPlaying ? "PLAY" : "PAUSA");
        drawBtMusicState();
    };
    cbs.onMenu  = []() {
        bt.end();
        returnToMenu();
    };
    buttons.setCallbacks(cbs);
}

// =====================================================
// BLUETOOTH - OLVIDAR VINCULOS
// =====================================================

// Un host emparejado cachea el descriptor HID y la tabla de servicios: se
// queda con las capacidades que vio la primera vez y no las vuelve a leer.
// Para que reconozca un set distinto hay que romper el vinculo de los DOS
// lados. Esto se encarga del lado del ESP32.

static void drawUnpairPrompt() {
    int n = bt.getBondedCount();
    char line1[24];
    snprintf(line1, sizeof(line1), "Vinculos: %d", n < 0 ? 0 : n);
    screen.showTextLines(line1, "[OK] borrar todos", "[MENU] salir", GC9A01A_YELLOW);
}

void enterBtUnpair() {
    speaker.stop();
    bt.begin();               // hace falta la radio encendida para consultar
    drawUnpairPrompt();
    itemLoopCallback = nullptr;

    ButtonActionCallbacks cbs;
    cbs.onOk = []() {
        int removed = bt.clearBonds();
        char line1[24];
        snprintf(line1, sizeof(line1), "Borrados: %d", removed < 0 ? 0 : removed);
        // La radio queda encendida y anunciando para poder emparejar de una vez
        screen.showTextLines(line1, "Olvidalo tambien", "en el telefono", GC9A01A_GREEN);
    };
    cbs.onMenu = []() {
        bt.end();
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

// Derecha avanza en el array y izquierda retrocede, para que el recorrido siga
// el orden en que estan declaradas las secciones en menuSections[].
static void onMenuLeft()   {
    currentSection = (currentSection - 1 + sectionCount) % sectionCount;
    currentItem = 0;
    Serial.print("[MENU] seccion: "); Serial.println(sections[currentSection].title);
    renderMenu();
}

static void onMenuRight()  {
    currentSection = (currentSection + 1) % sectionCount;
    currentItem = 0;
    Serial.print("[MENU] seccion: "); Serial.println(sections[currentSection].title);
    renderMenu();
}

static void onMenuUp()     {
    int count = sections[currentSection].itemCount;
    currentItem = (currentItem - 1 + count) % count;
    Serial.print("[MENU] item: "); Serial.println(sections[currentSection].items[currentItem].name);
    renderMenu();
}

static void onMenuDown()   {
    int count = sections[currentSection].itemCount;
    currentItem = (currentItem + 1) % count;
    Serial.print("[MENU] item: "); Serial.println(sections[currentSection].items[currentItem].name);
    renderMenu();
}

static void onMenuOk()     {
    itemLoopCallback = nullptr;
    buttons.setCallbacks(getItemCallbacks());
    const MenuItem* item = &sections[currentSection].items[currentItem];
    Serial.print("[MENU] entrar: ");
    Serial.print(sections[currentSection].title);
    Serial.print(" > ");
    Serial.println(item->name);
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

// Redibuja lo que hay ahora mismo en pantalla sin cambiar de estado. Tras un
// reinit del display la pantalla queda en negro, asi que hay que repintar:
// si hay una herramienta activa se reejecuta su onEnter (que la redibuja), y
// si estamos en el menu se redibuja el menu en su seccion/item actual.
void redrawCurrentState() {
    if (itemLoopCallback != nullptr) {
        const MenuItem* item = &sections[currentSection].items[currentItem];
        if (item->onEnter) item->onEnter();
    } else {
        renderMenu();
    }
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
    // Red de seguridad: una herramienta que desacople el stick (espejo o
    // gestos) no puede dejar el menu asi, o se quedaria sin forma de navegar
    // ni de salir. Se restaura el comportamiento por defecto al volver.
    joystick.setGesturesEnabled(true);
    joystick.mirror(&buttons);
    buttons.setCallbacks(getMenuCallbacks());
    renderMenu();
}

ButtonActionCallbacks getItemCallbacks() {
    ButtonActionCallbacks cbs;
    cbs.onOk   = onItemOk;
    cbs.onMenu = returnToMenu;
    return cbs;
}
