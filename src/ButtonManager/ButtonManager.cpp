#include "ButtonManager.h"

static const unsigned long DEBOUNCE_MS = 50;

ButtonManager::ButtonManager(ButtonPinConfig config) : config(config) {}

void ButtonManager::begin() {
    pinMode(config.bMenuPin, INPUT_PULLUP);
    pinMode(config.bAPin, INPUT_PULLUP);
    pinMode(config.bBPin, INPUT_PULLUP);
    pinMode(config.bUpPin, INPUT_PULLUP);
    pinMode(config.bDownPin, INPUT_PULLUP);
    pinMode(config.bLeftPin, INPUT_PULLUP);
    pinMode(config.bRightPin, INPUT_PULLUP);
    pinMode(config.bOkPin, INPUT_PULLUP);
}

void ButtonManager::update() {
    processInputs();
    executeActions();
}

static bool readAndDebounce(int pin, bool& lastState, bool& pressed, unsigned long& debounceTime, unsigned long now) {
    bool reading = digitalRead(pin) == LOW;
    if (reading != lastState) {
        debounceTime = now;
    }
    if ((now - debounceTime) > DEBOUNCE_MS) {
        if (reading != pressed) {
            pressed = reading;
            lastState = reading;
            return pressed;
        }
    }
    lastState = reading;
    return false;
}

void ButtonManager::processInputs() {
    static unsigned long dTime1 = 0, dTime2 = 0, dTime3 = 0;
    static unsigned long dTime4 = 0, dTime5 = 0, dTime6 = 0, dTime7 = 0, dTime8 = 0;
    unsigned long now = millis();

    if (readAndDebounce(config.bMenuPin, lastButton1State, menuPressed, dTime1, now) && menuPressed && menuCallback)
        menuCallback();
    if (readAndDebounce(config.bAPin,    lastAState,      aPressed,   dTime2, now) && aPressed && aCallback)
        aCallback();
    if (readAndDebounce(config.bBPin,    lastBState,      bPressed,   dTime3, now) && bPressed && bCallback)
        bCallback();
    if (readAndDebounce(config.bUpPin,   lastUpState,     upPressed,  dTime4, now) && upPressed && upCallback)
        upCallback();
    if (readAndDebounce(config.bDownPin, lastDownState,   downPressed,dTime5, now) && downPressed && downCallback)
        downCallback();
    if (readAndDebounce(config.bLeftPin, lastLeftState,   leftPressed,dTime6, now) && leftPressed && leftCallback)
        leftCallback();
    if (readAndDebounce(config.bRightPin,lastRightState,  rightPressed,dTime7, now) && rightPressed && rightCallback)
        rightCallback();
    if (readAndDebounce(config.bOkPin,   lastOkState,     okPressed,  dTime8, now) && okPressed && okCallback)
        okCallback();
}

void ButtonManager::executeActions() {

}

void ButtonManager::onMenu(void (*callback)())   { menuCallback   = callback; }
void ButtonManager::onA(void (*callback)())      { aCallback      = callback; }
void ButtonManager::onB(void (*callback)())      { bCallback      = callback; }
void ButtonManager::onUp(void (*callback)())     { upCallback     = callback; }
void ButtonManager::onDown(void (*callback)())   { downCallback   = callback; }
void ButtonManager::onLeft(void (*callback)())   { leftCallback   = callback; }
void ButtonManager::onRight(void (*callback)())  { rightCallback  = callback; }
void ButtonManager::onOk(void (*callback)())     { okCallback     = callback; }

void ButtonManager::setCallbacks(ButtonActionCallbacks callbacks) {
    menuCallback   = callbacks.onMenu;
    aCallback      = callbacks.onA;
    bCallback      = callbacks.onB;
    upCallback     = callbacks.onUp;
    downCallback   = callbacks.onDown;
    leftCallback   = callbacks.onLeft;
    rightCallback  = callbacks.onRight;
    okCallback     = callbacks.onOk;
}
