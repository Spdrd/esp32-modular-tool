#pragma once
#include <Arduino.h>

class DiceManager {
public:
    DiceManager();

    void increaseMax();
    void decreaseMax();
    void roll();

    int getMax() const { return maxValue; }
    int getResult() const { return result; }
    bool hasRolled() const { return rolled; }

private:
    int maxValue;
    int result;
    bool rolled;
};
