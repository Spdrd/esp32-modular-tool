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

// =====================================================
// MENU DATA
// =====================================================

static void enterTestMenu();
static void enterTestA();
static void enterTestB();
static void enterTestUp();
static void enterTestDown();
static void enterTestLeft();
static void enterTestRight();
static void enterTestOk();
static void enterTestAll();
static void enterFillRed();
static void enterFillGreen();
static void enterFillBlue();
static void enterClear();
static void enterVersion();
static void enterCredits();
static void enterSnake();
static void enterSimon();
static void enterCronometro();
static void enterDado();
static void enterCanvas();
static void enterTimer();

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
};

static const MenuItem toolsItems[] = {
    {"Cronometro", enterCronometro},
    {"Dado",       enterDado},
    {"Canvas",     enterCanvas},
    {"Temporizador",enterTimer},
};

static const MenuSection menuSections[] = {
    {"Tests",   testItems,   9},
    {"Pantalla",displayItems,4},
    {"Info",    infoItems,   2},
    {"Juegos",  gamesItems,  2},
    {"Herramientas",toolsItems,4},
};

const MenuSection* sections = menuSections;
int sectionCount = sizeof(menuSections) / sizeof(menuSections[0]);

int currentSection = 0;
int currentItem = 0;
static void (*itemLoopCallback)() = nullptr;

// =====================================================
// ON ENTER FUNCTIONS
// =====================================================

static void showMessageStay(const char* msg, uint16_t color) {
    screen.showTextLines(msg, "", "", color);
}

static void enterTestMenu()  { showMessageStay("Menu",  GC9A01A_BLUE);    }
static void enterTestA()     { showMessageStay("A",     GC9A01A_GREEN);   }
static void enterTestB()     { showMessageStay("B",     GC9A01A_RED);     }
static void enterTestUp()    { showMessageStay("UP",    GC9A01A_CYAN);    }
static void enterTestDown()  { showMessageStay("DOWN",  GC9A01A_YELLOW);  }
static void enterTestLeft()  { showMessageStay("LEFT",  GC9A01A_MAGENTA); }
static void enterTestRight() { showMessageStay("RIGHT", GC9A01A_WHITE);   }
static void enterTestOk()    { showMessageStay("OK",    GC9A01A_GREEN);   }
static void enterTestAll()   { showMessageStay("ALL",   GC9A01A_ORANGE);  }

static void enterFillRed()   { screen.tft.fillScreen(GC9A01A_RED);   }
static void enterFillGreen() { screen.tft.fillScreen(GC9A01A_GREEN); }
static void enterFillBlue()  { screen.tft.fillScreen(GC9A01A_BLUE);  }
static void enterClear()     { screen.tft.fillScreen(GC9A01A_BLACK); }

static void enterVersion()   { showMessageStay("v1.0 - 2026", GC9A01A_CYAN); }
static void enterCredits()   { showMessageStay("By J. Diego", GC9A01A_BLUE); }

// =====================================================
// SIMON
// =====================================================

static void drawSimonState() {
    screen.drawSimon(
        simon.getHighlight(),
        simon.getScore(),
        (int)simon.getState(),
        simon.isFlashOn()
    );
}

static void simonLoop() {
    if (simon.update()) {
        drawSimonState();
    }
}

static void enterSimon() {
    simon.reset();
    drawSimonState();
    itemLoopCallback = simonLoop;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { simon.pressButton(0); drawSimonState(); };
    cbs.onDown  = []() { simon.pressButton(1); drawSimonState(); };
    cbs.onLeft  = []() { simon.pressButton(2); drawSimonState(); };
    cbs.onRight = []() { simon.pressButton(3); drawSimonState(); };
    cbs.onOk    = []() {
        if (simon.getState() == SimonGame::LOSE) {
            simon.reset();
            drawSimonState();
        }
    };
    cbs.onMenu  = returnToMenu;
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

static void enterSnake() {
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

static void enterCronometro() {
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

static void enterDado() {
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

static void enterCanvas() {
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

static void enterTimer() {
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
    buttons.setCallbacks(getMenuCallbacks());
    renderMenu();
}

ButtonActionCallbacks getItemCallbacks() {
    ButtonActionCallbacks cbs;
    cbs.onOk   = onItemOk;
    cbs.onMenu = returnToMenu;
    return cbs;
}
