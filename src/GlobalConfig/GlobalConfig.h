#pragma once

#include "ScreenManager/ScreenManager.h"
#include "ButtonManager/ButtonManager.h"
#include "SnakeGame/SnakeGame.h"
#include "CronoManager/CronoManager.h"
#include "DiceManager/DiceManager.h"
#include "CanvasManager/CanvasManager.h"

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

// --- MENU STATE ---
extern int currentSection;
extern int currentItem;
extern const MenuSection* sections;
extern int sectionCount;

// --- FUNCTIONS ---
void renderMenu();
ButtonActionCallbacks getMenuCallbacks();
ButtonActionCallbacks getItemCallbacks();
void returnToMenu();
void itemLoopUpdate();
