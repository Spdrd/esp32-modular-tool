#include "DiceManager.h"

DiceManager::DiceManager() {
    maxValue = 6;
    result = 0;
    rolled = false;
}

void DiceManager::increaseMax() {
    maxValue++;
    if (maxValue > 100) maxValue = 100;
    rolled = false;
}

void DiceManager::decreaseMax() {
    maxValue--;
    if (maxValue < 2) maxValue = 2;
    rolled = false;
}

void DiceManager::roll() {
    result = random(1, maxValue + 1);
    rolled = true;
}
