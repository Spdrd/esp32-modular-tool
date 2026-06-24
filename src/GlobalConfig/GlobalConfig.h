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
#include "audio/OuterWilds.h"
#include "audio/Riptide.h"

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
extern ScreenManager      screen;
extern ButtonManager      buttons;
extern SnakeGame          snake;
extern CronoManager       crono;
extern DiceManager        dice;
extern CanvasManager      canvas;
extern TimerManager       timer;
extern SimonGame          simon;
extern TetrisGame         tetris;
extern Game2048           game2048;
extern MorseCode          morse;
extern SpeakerManager     speaker;
extern LinternaManager    linterna;

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
