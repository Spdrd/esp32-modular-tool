#pragma once
#include <Arduino.h>
#include <Adafruit_GC9A01A.h>

class CronoManager {
public:
    CronoManager();

    void reset();
    void toggle();
    void markLap();
    void render(Adafruit_GC9A01A& tft);
    void loop(Adafruit_GC9A01A& tft);
    bool isRunning() const { return running; }
    unsigned long getElapsed() const;

private:
    static const int MAX_LAPS = 10;

    unsigned long startMs;
    unsigned long pauseMs;
    bool running;
    int lapCount;
    unsigned long laps[MAX_LAPS];
    unsigned long lastRender;

    void formatTime(unsigned long ms, char* buf) const;
};
