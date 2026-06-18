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
static void enterCronometro();
static void enterDado();
static void enterCanvas();

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
};

static const MenuItem toolsItems[] = {
    {"Cronometro", enterCronometro},
    {"Dado",       enterDado},
    {"Canvas",     enterCanvas},
};

static const MenuSection menuSections[] = {
    {"Tests",   testItems,   9},
    {"Pantalla",displayItems,4},
    {"Info",    infoItems,   2},
    {"Juegos",  gamesItems,  1},
    {"Herramientas",toolsItems,3},
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
// SNAKE
// =====================================================

static void snakeUp()    { snake.setDirection(0, -1); }
static void snakeDown()  { snake.setDirection(0, 1); }
static void snakeLeft()  { snake.setDirection(-1, 0); }
static void snakeRight() { snake.setDirection(1, 0); }

static void snakeOk() {
    if (snake.isGameOver()) {
        snake.reset();
        snake.render(screen.tft);
    }
}

static void snakeLoop() {
    snake.update(screen.tft);
}

static void enterSnake() {
    snake.reset();
    snake.render(screen.tft);
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

static void cronoLoop() { crono.loop(screen.tft); }

static void enterCronometro() {
    crono.reset();
    crono.render(screen.tft);
    itemLoopCallback = cronoLoop;

    ButtonActionCallbacks cbs;
    cbs.onOk   = []() { crono.toggle(); crono.render(screen.tft); };
    cbs.onA    = []() { crono.markLap(); crono.render(screen.tft); };
    cbs.onB    = []() { crono.reset(); crono.render(screen.tft); };
    cbs.onMenu = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// DADO
// =====================================================

static void enterDado() {
    dice.render(screen.tft);
    itemLoopCallback = nullptr;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { dice.increaseMax(); dice.render(screen.tft); };
    cbs.onDown  = []() { dice.decreaseMax(); dice.render(screen.tft); };
    cbs.onOk    = []() { dice.roll(); dice.render(screen.tft); };
    cbs.onMenu  = returnToMenu;
    buttons.setCallbacks(cbs);
}

// =====================================================
// CANVAS
// =====================================================

static void enterCanvas() {
    canvas.render(screen.tft);
    itemLoopCallback = nullptr;

    ButtonActionCallbacks cbs;
    cbs.onUp    = []() { canvas.moveCursor(0, -1); canvas.render(screen.tft); };
    cbs.onDown  = []() { canvas.moveCursor(0, 1); canvas.render(screen.tft); };
    cbs.onLeft  = []() { canvas.moveCursor(-1, 0); canvas.render(screen.tft); };
    cbs.onRight = []() { canvas.moveCursor(1, 0); canvas.render(screen.tft); };
    cbs.onOk    = []() { canvas.paint(); canvas.render(screen.tft); };
    cbs.onA     = []() { canvas.nextColor(); canvas.render(screen.tft); };
    cbs.onB     = []() { canvas.reset(); canvas.render(screen.tft); };
    cbs.onMenu  = returnToMenu;
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
    const MenuItem* item = &sections[currentSection].items[currentItem];
    if (item->onEnter) item->onEnter();
    buttons.setCallbacks(getItemCallbacks());
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
