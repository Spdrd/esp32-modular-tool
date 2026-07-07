#include "FlappyGame.h"
#include <stdlib.h>

FlappyGame::FlappyGame() { reset(); }

void FlappyGame::reset() {
    birdY = H / 2.0f; velY = 0;
    score = 0; gameOver = false; started = false; lastMs = 0;
    nextPipe();
}

void FlappyGame::nextPipe() {
    pipeX = W;
    gapY  = 38 + rand() % (H - 80 - GAP);
}

void FlappyGame::flap() {
    if (gameOver) return;
    started = true;
    velY = -4.8f;
}

bool FlappyGame::update() {
    if (!started || gameOver) return false;
    unsigned long now = millis();
    if (now - lastMs < 20) return false;
    lastMs = now;

    velY  += 0.42f;
    birdY += velY;

    if (birdY - BIRD_R <= 0 || birdY + BIRD_R >= H) { gameOver = true; return true; }

    pipeX -= 2;
    if (pipeX + PIPE_W < 0) { nextPipe(); score++; }

    // Collision
    if (BIRD_X + BIRD_R > pipeX && BIRD_X - BIRD_R < pipeX + PIPE_W) {
        if ((int)birdY - BIRD_R < gapY || (int)birdY + BIRD_R > gapY + GAP) {
            gameOver = true;
        }
    }

    return true;
}
