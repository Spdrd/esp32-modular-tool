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

bool CronoManager::shouldRender() {
    if (!running) return false;
    unsigned long now = millis();
    if (now - lastRender > 50) {
        lastRender = now;
        return true;
    }
    return false;
}
