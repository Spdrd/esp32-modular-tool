#include "CronoManager.h"

CronoManager::CronoManager() {
    reset();
}

void CronoManager::reset() {
    running = false;
    startMs = 0;
    pauseMs = 0;
    lapCount = 0;
    lastRender = 0;
}

void CronoManager::toggle() {
    if (running) {
        pauseMs = millis() - startMs;
        running = false;
    } else {
        if (pauseMs == 0) {
            lapCount = 0;
            startMs = millis();
        } else {
            startMs = millis() - pauseMs;
        }
        running = true;
    }
}

void CronoManager::markLap() {
    if (!running) return;
    if (lapCount >= MAX_LAPS) return;
    laps[lapCount++] = millis() - startMs;
}

unsigned long CronoManager::getElapsed() const {
    return running ? (millis() - startMs) : pauseMs;
}

void CronoManager::formatTime(unsigned long ms, char* buf) const {
    unsigned long t = ms;
    int m = t / 60000;
    int s = (t % 60000) / 1000;
    int c = (t % 1000) / 10;
    sprintf(buf, "%02d:%02d.%02d", m, s, c);
}

void CronoManager::render(Adafruit_GC9A01A& tft) {
    tft.fillScreen(GC9A01A_BLACK);

    char buf[16];
    formatTime(getElapsed(), buf);

    tft.setTextSize(3);
    tft.setTextColor(GC9A01A_GREEN);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor((tft.width() - w) / 2, 35);
    tft.println(buf);

    tft.drawFastHLine(10, 70, 220, GC9A01A_DARKGREY);

    tft.setTextSize(1);
    tft.setCursor(10, 80);
    tft.setTextColor(running ? GC9A01A_GREEN : GC9A01A_YELLOW);
    tft.println(running ? "CORRIENDO" : "DETENIDO");

    int lapY = 95;
    int start = lapCount > 6 ? lapCount - 6 : 0;
    for (int i = start; i < lapCount; i++) {
        formatTime(laps[i], buf);
        tft.setCursor(10, lapY);
        tft.setTextColor(GC9A01A_CYAN);
        tft.print("V");
        tft.print(i + 1);
        tft.print(": ");
        tft.println(buf);
        lapY += 12;
    }

    tft.setTextColor(GC9A01A_LIGHTGREY);
    tft.setCursor(10, 220);
    tft.print("[OK] ");
    tft.print(running ? "Pausa" : "Inicia");
    tft.print("  [A] Marca  [B] Reset");
}

void CronoManager::loop(Adafruit_GC9A01A& tft) {
    if (running) {
        unsigned long now = millis();
        if (now - lastRender > 50) {
            lastRender = now;
            render(tft);
        }
    }
}
