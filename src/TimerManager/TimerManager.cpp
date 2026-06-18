#include "TimerManager.h"

TimerManager::TimerManager() {
    reset();
}

void TimerManager::reset() {
    state = SETTING;
    minutes = 1;
    seconds = 0;
    field = 0;
    totalMs = 0;
    startTime = 0;
    pausedRemaining = 0;
    flashTimer = 0;
    flashOn = false;
}

void TimerManager::up() {
    if (state != SETTING) return;
    if (field == 0) {
        minutes++;
        if (minutes > 99) minutes = 99;
    } else {
        seconds += 5;
        if (seconds >= 60) seconds = 55;
    }
}

void TimerManager::down() {
    if (state != SETTING) return;
    if (field == 0) {
        minutes--;
        if (minutes < 0) minutes = 0;
    } else {
        seconds -= 5;
        if (seconds < 0) seconds = 0;
    }
}

void TimerManager::nextField() {
    if (state != SETTING) return;
    field = 1 - field;
}

void TimerManager::ok() {
    if (state == SETTING) {
        totalMs = (unsigned long)minutes * 60000 + seconds * 1000;
        if (totalMs == 0) return;
        startTime = millis();
        pausedRemaining = 0;
        state = RUNNING;
    } else if (state == RUNNING) {
        pausedRemaining = getRemainingMs();
        state = SETTING;
        minutes = pausedRemaining / 60000;
        seconds = (pausedRemaining % 60000) / 1000;
    }
}

unsigned long TimerManager::getRemainingMs() const {
    if (state == SETTING) return (unsigned long)minutes * 60000 + seconds * 1000;
    if (state == DONE) return 0;
    unsigned long elapsed = millis() - startTime;
    if (elapsed >= totalMs) return 0;
    return totalMs - elapsed;
}

bool TimerManager::update() {
    unsigned long now = millis();
    if (state == RUNNING) {
        if (getRemainingMs() == 0) {
            state = DONE;
            flashOn = true;
            flashTimer = now;
            return true;
        }
        return true;
    }
    if (state == DONE) {
        if (now - flashTimer > 500) {
            flashTimer = now;
            flashOn = !flashOn;
            return true;
        }
    }
    return false;
}
