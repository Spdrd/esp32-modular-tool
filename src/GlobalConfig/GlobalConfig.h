#pragma once

#include "ScreenManager/ScreenManager.h"
#include "ButtonManager/ButtonManager.h"
#include "SnakeGame/SnakeGame.h"
#include "CronoManager/CronoManager.h"
#include "DiceManager/DiceManager.h"
#include "CanvasManager/CanvasManager.h"
#include "TimerManager/TimerManager.h"
#include "SimonGame/SimonGame.h"
#include "TetrisGame/TetrisGame.h"
#include "Game2048/Game2048.h"
#include "MorseCode/MorseCode.h"
#include "SpeakerManager/SpeakerManager.h"
#include "LinternaManager/LinternaManager.h"
#include "audio/Aerodynamic.h"
#include "audio/GetLucky.h"
#include "audio/NeverGonnaGiveYouUp.h"
#include "audio/Kids.h"
#include "audio/AdultsAreTalking.h"
#include "audio/HarderBetterFasterStronger.h"

#define SPEAKER_PIN     25
#define SPEAKER_CHANNEL  0

// --- SCREEN PINS ---
#define TFT_CS    5
#define TFT_RST   22
#define TFT_DC    21
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_MISO  -1

// --- BUTTON PINS ---
#define BTN_MENU  16
#define BTN_A     4
#define BTN_B     15
#define BTN_UP    27
#define BTN_DOWN  13
#define BTN_LEFT  14
#define BTN_RIGHT 26
#define BTN_OK    12

// -- SPREAKER PINS ---
#define SPK_PIN   25

// --- LINTERNA PIN ---
#define LINTERNA_PIN 32

// --- MENU STRUCTS ---
struct MenuItem {
    const char* name;
    void (*onEnter)();
};

struct MenuSection {
    const char* title;
    const MenuItem* items;
    int itemCount;
};

// --- EXTERN INSTANCES ---
extern ScreenManager screen;
extern ButtonManager buttons;
extern SnakeGame snake;
extern CronoManager crono;
extern DiceManager dice;
extern CanvasManager canvas;
extern TimerManager timer;
extern SimonGame simon;
extern TetrisGame tetris;
extern Game2048 game2048;
extern MorseCode morse;
extern SpeakerManager speaker;
extern LinternaManager linterna;

// --- MENU STATE ---
extern int currentSection;
extern int currentItem;
extern const MenuSection* sections;
extern int sectionCount;

// --- MENU NAVIGATION ---
void renderMenu();
ButtonActionCallbacks getMenuCallbacks();
ButtonActionCallbacks getItemCallbacks();
void returnToMenu();
void itemLoopUpdate();

// --- ENTER FUNCTIONS ---
void enterTestMenu();
void enterTestA();
void enterTestB();
void enterTestUp();
void enterTestDown();
void enterTestLeft();
void enterTestRight();
void enterTestOk();
void enterTestAll();
void enterFillRed();
void enterFillGreen();
void enterFillBlue();
void enterClear();
void enterVersion();
void enterCredits();
void enterSnake();
void enterSimon();
void enterTetris();
void enterGame2048();
void enterMorse();
void enterCronometro();
void enterDado();
void enterCanvas();
void enterTimer();
void enterSynth();
void enterLinterna();
void enterSong(const Song& song);

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
};

static const MenuItem musicItems[] = {
    {"Aerodynamic",       []() { enterSong(SONG_AERODYNAMIC); }},
    {"Get Lucky",         []() { enterSong(SONG_GETLUCKY); }},
    {"H-B-F-S",           []() { enterSong(SONG_HBF); }},
    {"Never Gonna",       []() { enterSong(SONG_NGGYU); }},
    {"Kids",              []() { enterSong(SONG_KIDS); }},
    {"Adults Are Talking",[]() { enterSong(SONG_ADULTS_ARE_TALKING); }},
};

static const MenuSection menuSections[] = {
    {"Tests",   testItems,   9},
    {"Pantalla",displayItems,4},
    {"Info",    infoItems,   2},
    {"Juegos",  gamesItems,  4},
    {"Herramientas",toolsItems,7},
    {"Musica",  musicItems,  6},
};

const MenuSection* sections = menuSections;
int sectionCount = sizeof(menuSections) / sizeof(menuSections[0]);

int currentSection = 0;
int currentItem = 0;
static void (*itemLoopCallback)() = nullptr;

// =====================================================
// SIMON
// =====================================================

// Notas clásicas Simon: UP=E5, DOWN=C5, LEFT=G4, RIGHT=A4, LOSE=E3
static const uint16_t SIMON_NOTES[4] = { E5, C5, G4, A4 };

static MusNote s_simonNote[1];
static Song    s_simonSong = { "Simon", s_simonNote, 1 };

// =====================================================
// SYNTH
// =====================================================

// Frecuencias para 4 octavas × 12 semitonos (octavas 3-6)
static const uint16_t SYNTH_FREQS[4][12] = {
    {131,139,147,156,165,175,185,196,208,220,233,247}, // oct 3
    {262,277,294,311,330,349,370,392,415,440,466,494}, // oct 4
    {523,554,587,622,659,698,740,784,831,880,932,988}, // oct 5
    {1047,1109,1175,1245,1319,1397,1480,1568,1661,1760,1865,1976}, // oct 6
};

static int  s_synthNote    = 0; // 0=C … 11=B
static int  s_synthOct     = 1; // índice en SYNTH_FREQS (0=oct3 … 3=oct6), arrancar en oct4
static bool s_synthPlaying = false;
static bool s_synthSustain = false;
static bool s_synthOkWas   = false;
