#include "SimonGame.h"

SimonGame::SimonGame() {
    reset();
}

void SimonGame::reset() {
    seqLen   = 0;
    showIdx  = 0;
    inputIdx = 0;
    score    = 0;
    highlight = -1;
    flashOn  = false;
    dirty    = true;
    state    = SHOWING;
    nextMs   = millis() + 1200;

    addStep();
    addStep();
}

void SimonGame::addStep() {
    if (seqLen < MAX_SEQ) {
        sequence[seqLen++] = random(4);
    }
}

void SimonGame::markDirty() {
    dirty = true;
}

bool SimonGame::update() {
    unsigned long now = millis();
    bool changed = dirty;
    dirty = false;

    switch (state) {
        case SHOWING:
            if (now >= nextMs) {
                if (highlight == -1) {
                    highlight = sequence[showIdx];
                    nextMs = now + 500;
                } else {
                    highlight = -1;
                    showIdx++;
                    if (showIdx >= seqLen) {
                        state    = WAITING;
                        inputIdx = 0;
                    } else {
                        nextMs = now + 250;
                    }
                }
                changed = true;
            }
            break;

        case WAITING:
            // Clear button flash after brief delay
            if (highlight != -1 && now >= nextMs) {
                highlight = -1;
                changed = true;
            }
            break;

        case WIN_STEP:
            if (now >= nextMs) {
                addStep();
                showIdx  = 0;
                highlight = -1;
                state    = SHOWING;
                nextMs   = now + 700;
                changed  = true;
            }
            break;

        case LOSE:
            if (now >= flashMs) {
                flashOn = !flashOn;
                flashMs = now + 350;
                changed = true;
            }
            break;
    }

    return changed;
}

bool SimonGame::pressButton(int btn) {
    if (state != WAITING) return false;

    highlight = btn;
    nextMs    = millis() + 350;
    dirty     = true;

    if (btn == sequence[inputIdx]) {
        inputIdx++;
        if (inputIdx >= seqLen) {
            score++;
            state  = WIN_STEP;
            nextMs = millis() + 800;
        }
    } else {
        state   = LOSE;
        flashOn = true;
        flashMs = millis() + 350;
    }

    return true;
}
