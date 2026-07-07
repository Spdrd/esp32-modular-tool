#include "PongGame.h"
#include <stdlib.h>
#include <math.h>

PongGame::PongGame() { reset(); }

void PongGame::reset() {
    playerY = H / 2; aiY = H / 2;
    pScore = 0; aScore = 0; gameOver = false; lastMs = 0;
    resetBall(1);
}

void PongGame::resetBall(int dir) {
    bx = W / 2.0f; by = H / 2.0f;
    vx = dir * 2.8f;
    vy = ((rand() % 5) - 2) * 0.9f;
    if (fabsf(vy) < 0.6f) vy = 0.9f;
}

void PongGame::moveUp()   { playerY = max(PAD_H / 2, playerY - 8); }
void PongGame::moveDown() { playerY = min(H - PAD_H / 2, playerY + 8); }

bool PongGame::update() {
    if (gameOver) return false;
    unsigned long now = millis();
    if (now - lastMs < 14) return false;
    lastMs = now;

    // AI tracks ball (imperfect: limited speed)
    if (aiY < (int)by - 3) aiY = min(H - PAD_H / 2, aiY + 3);
    else if (aiY > (int)by + 3) aiY = max(PAD_H / 2, aiY - 3);

    bx += vx; by += vy;

    // Top / bottom walls
    if (by <= 0)               { by = 0;              vy =  fabsf(vy); }
    if (by >= H - BALL_S)      { by = H - BALL_S;     vy = -fabsf(vy); }

    // AI paddle (left, x = 6)
    if (vx < 0 && bx <= 6 + PAD_W &&
        by + BALL_S >= aiY - PAD_H / 2 && by <= aiY + PAD_H / 2) {
        vx = fabsf(vx) + 0.15f;
        bx = 6 + PAD_W;
    }

    // Player paddle (right, x = W-6-PAD_W)
    int px = W - 6 - PAD_W;
    if (vx > 0 && bx + BALL_S >= px &&
        by + BALL_S >= playerY - PAD_H / 2 && by <= playerY + PAD_H / 2) {
        float rel = (by + BALL_S / 2.0f - playerY) / (float)(PAD_H / 2);
        vx = -(fabsf(vx) + 0.15f);
        vy = rel * 3.5f;
        bx = px - BALL_S;
    }

    // Score
    if (bx < 0) {
        pScore++;
        if (pScore >= WIN_SCORE) { gameOver = true; return true; }
        resetBall(1);
    }
    if (bx > W) {
        aScore++;
        if (aScore >= WIN_SCORE) { gameOver = true; return true; }
        resetBall(-1);
    }

    return true;
}
