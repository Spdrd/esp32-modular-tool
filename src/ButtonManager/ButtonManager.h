#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

#include <Arduino.h>

struct ButtonPinConfig
{
    int bMenuPin;
    int bAPin;
    int bBPin;
    int bUpPin;
    int bDownPin;
    int bLeftPin;
    int bRightPin;
    int bOkPin;
};

struct ButtonActionCallbacks
{
    void (*onMenu)()  = nullptr;
    void (*onA)()     = nullptr;
    void (*onB)()     = nullptr;
    void (*onUp)()    = nullptr;
    void (*onDown)()  = nullptr;
    void (*onLeft)()  = nullptr;
    void (*onRight)() = nullptr;
    void (*onOk)()    = nullptr;
};

class ButtonManager {
public:
    ButtonManager(ButtonPinConfig config);

    void begin();
    void update();

    void onMenu(void (*callback)());
    void onA(void (*callback)());
    void onB(void (*callback)());
    void onUp(void (*callback)());
    void onDown(void (*callback)());
    void onLeft(void (*callback)());
    void onRight(void (*callback)());
    void onOk(void (*callback)());

    void setCallbacks(ButtonActionCallbacks callbacks);

    bool isOkDown()    const { return digitalRead(config.bOkPin)    == LOW; }
    bool isUpDown()    const { return digitalRead(config.bUpPin)    == LOW; }
    bool isDownDown()  const { return digitalRead(config.bDownPin)  == LOW; }
    bool isLeftDown()  const { return digitalRead(config.bLeftPin)  == LOW; }
    bool isRightDown() const { return digitalRead(config.bRightPin) == LOW; }
    bool isADown()     const { return digitalRead(config.bAPin)     == LOW; }
    bool isBDown()     const { return digitalRead(config.bBPin)     == LOW; }

private:
    ButtonPinConfig config;

    bool menuPressed   = false;
    bool aPressed      = false;
    bool bPressed      = false;
    bool upPressed     = false;
    bool downPressed   = false;
    bool leftPressed   = false;
    bool rightPressed  = false;
    bool okPressed     = false;

    bool lastButton1State = false;
    bool lastAState       = false;
    bool lastBState       = false;
    bool lastUpState      = false;
    bool lastDownState    = false;
    bool lastLeftState    = false;
    bool lastRightState   = false;
    bool lastOkState      = false;

    int button2HoldCounter = 0;
    int button3HoldCounter = 0;

    void (*menuCallback)()  = nullptr;
    void (*aCallback)()     = nullptr;
    void (*bCallback)()     = nullptr;
    void (*upCallback)()    = nullptr;
    void (*downCallback)()  = nullptr;
    void (*leftCallback)()  = nullptr;
    void (*rightCallback)() = nullptr;
    void (*okCallback)()    = nullptr;

    void processInputs();
    void executeActions();
};

#endif
