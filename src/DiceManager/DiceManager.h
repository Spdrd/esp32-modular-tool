#pragma once
#include <Arduino.h>
#include <Adafruit_GC9A01A.h>

class DiceManager {
public:
    DiceManager();

    void increaseMax();
    void decreaseMax();
    void roll();
    void render(Adafruit_GC9A01A& tft);

    int getMax() const { return maxValue; }
    int getResult() const { return result; }
    bool hasRolled() const { return rolled; }

private:
    int maxValue;
    int result;
    bool rolled;
};
