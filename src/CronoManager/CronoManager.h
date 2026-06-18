#pragma once
#include <Arduino.h>

class CronoManager {
public:
    CronoManager();

    void reset();
    void toggle();
    void markLap();
    bool shouldRender();

    bool isRunning() const { return running; }
    unsigned long getElapsed() const;
    int getLapCount() const { return lapCount; }
    unsigned long getLap(int i) const { return laps[i]; }

private:
    static const int MAX_LAPS = 10;

    unsigned long startMs;
    unsigned long pauseMs;
    bool running;
    int lapCount;
    unsigned long laps[MAX_LAPS];
    unsigned long lastRender;
};
