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

void DiceManager::render(Adafruit_GC9A01A& tft) {
    tft.fillScreen(GC9A01A_BLACK);

    char buf[16];
    sprintf(buf, "DADO (d%d)", maxValue);

    int16_t x1, y1;
    uint16_t w, h;
    tft.setTextSize(2);
    tft.setTextColor(GC9A01A_WHITE);
    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 15);
    tft.println(buf);

    tft.drawRoundRect(50, 50, 140, 100, 12, GC9A01A_DARKGREY);

    if (rolled) {
        sprintf(buf, "%d", result);
        tft.setTextSize(6);
        tft.setTextColor(GC9A01A_YELLOW);
        tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, 70);
        tft.println(buf);
    } else {
        tft.setTextSize(4);
        tft.setTextColor(GC9A01A_DARKGREY);
        tft.getTextBounds("?", 0, 0, &x1, &y1, &w, &h);
        tft.setCursor((tft.width() - w) / 2, 75);
        tft.println("?");
    }

    tft.setTextSize(1);
    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(10, 220);
    tft.println("[UP/DN] Caras  [OK] Tirar");
}
