#pragma once
#include <Arduino.h>

class TimerManager {
public:
    enum State { SETTING, RUNNING, DONE };

    TimerManager();

    void up();
    void down();
    void nextField();
    void ok();
    void reset();
    bool update();

    State getState() const { return state; }
    int getMinutes() const { return minutes; }
    int getSeconds() const { return seconds; }
    int getField() const { return field; }
    unsigned long getRemainingMs() const;
    bool isFlashOn() const { return flashOn; }

private:
    State state;
    int minutes;
    int seconds;
    int field;
    unsigned long totalMs;
    unsigned long startTime;
    unsigned long pausedRemaining;
    unsigned long flashTimer;
    bool flashOn;
};
