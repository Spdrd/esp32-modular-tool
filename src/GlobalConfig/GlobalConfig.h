#pragma once

#include "ScreenManager/ScreenManager.h"
#include "ButtonManager/ButtonManager.h"
#include "JoystickManager/JoystickManager.h"
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
#include "EspNowManager/EspNowManager.h"
#include "EspNowLedManager/EspNowLedManager.h"
#include "BluetoothManager/BluetoothManager.h"
#include "PongGame/PongGame.h"
#include "BreakoutGame/BreakoutGame.h"
#include "FlappyGame/FlappyGame.h"
#include "InvadersGame/InvadersGame.h"
#include "MinesweeperGame/MinesweeperGame.h"
#include "DoomGame/DoomGame.h"
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
#define BTN_LEFT  12
#define BTN_RIGHT 26
#define BTN_OK    14

// -- SPREAKER PINS ---
#define SPK_PIN   25

// --- LINTERNA PIN ---
#define LINTERNA_PIN 32

// --- JOYSTICK (potenciometros 0-3.3V) ---
// Tienen que ser pines de ADC1 (GPIO 32-39): el ADC2 deja de funcionar con el
// WiFi encendido y aqui se usa ESP-NOW. Los GPIO 34/35 ademas son solo de
// entrada, asi que no hay riesgo de excitarlos por error.
#define JOY_X_PIN   34
#define JOY_Y_PIN   35
// Boton integrado del stick. Va en el 33 y no en otro ADC1 porque el switch
// cierra a masa y necesita pull-up interno, que los GPIO 34-39 no tienen.
#define JOY_SW_PIN  33
#define JOY_INVERT_X true
#define JOY_INVERT_Y false

// --- CAM CAR (ESP-NOW) ---
// MAC del vehiculo ESP32-S3
#define CAMCAR_MAC {0x80, 0xB5, 0x4E, 0xC6, 0xF9, 0x1C}

// --- TIRA LED (ESP-NOW) ---
// Broadcast por defecto: el receptor lo acepta y no hace falta conocer su MAC.
// Para envio dirigido, pon aqui la MAC que el receptor imprime al arrancar
// ("MAC de este receptor: ..."). Dirigido da ACK real de ese equipo; el ACK
// de broadcast no confirma que alguien haya escuchado.
#define LEDSTRIP_MAC {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

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
extern JoystickManager    joystick;
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
extern EspNowManager      camCar;
extern EspNowLedManager   ledStrip;
extern BluetoothManager   bt;
extern PongGame           pong;
extern BreakoutGame       breakout;
extern FlappyGame         flappy;
extern InvadersGame       invaders;
extern MinesweeperGame    minesweeper;
extern DoomGame           doom;

// --- ENTRADA COMBINADA (cruceta + stick) ---
// El espejo del joystick solo reproduce callbacks de flanco. Todo lo que lea
// estado sostenido (juegos, secuenciadores, Cam Car...) debe usar estos
// helpers, o el stick quedaria muerto en esas pantallas.
inline bool inputUp()    { return buttons.isUpDown()    || joystick.isUpDown();    }
inline bool inputDown()  { return buttons.isDownDown()  || joystick.isDownDown();  }
inline bool inputLeft()  { return buttons.isLeftDown()  || joystick.isLeftDown();  }
inline bool inputRight() { return buttons.isRightDown() || joystick.isRightDown(); }

// OK sostenido NO incluye el boton del stick a proposito: alli la duracion de
// la pulsacion es lo que elige el gesto (OK/A/B/MENU), asi que mantenerlo para
// lanzar A tambien contaria como OK pulsado un segundo entero. En Morse eso
// seria una raya y en los teclados BT un SPACE continuo.
// El boton del stick actua por callbacks; para el OK sostenido esta el fisico.
inline bool inputOk()    { return buttons.isOkDown(); }

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
void enterTestJoystick();
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
void enterSirena();
void enterCamCar();
void enterLedStrip();
void enterTecladoWasd();
void enterTecladoMinecraft();
void enterMusicControl();
void enterBtMouse();
void enterBtUnpair();
void enterPong();
void enterBreakout();
void enterFlappy();
void enterInvaders();
void enterMinesweeper();
void enterDoom();
void enterSong(const Song& song);
