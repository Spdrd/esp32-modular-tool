#include "BreakoutGame.h"
#include <stdlib.h>
#include <math.h>

static const uint16_t ROW_COLORS[] = { 0, 1, 2, 3, 4 }; // index into palette in ScreenManager

BreakoutGame::BreakoutGame() { reset(); }

int BreakoutGame::getBrickColor(int r) const { return r; }

void BreakoutGame::reset() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            bricks[r][c] = true;
    bricksLeft = ROWS * COLS;
    score = 0; lives = 3;
    padX = (W - PAD_W) / 2;
    launched = false; lastMs = 0;
    resetBall();
}

void BreakoutGame::resetBall() {
    bx = padX + PAD_W / 2.0f;
    by = PAD_Y - BALL_R * 2 - 1;
    vx = (rand() % 2 == 0 ? 1 : -1) * 2.2f;
    vy = -2.6f;
    launched = false;
}

void BreakoutGame::moveLeft() {
    padX = max(0, padX - 7);
    if (!launched) bx = padX + PAD_W / 2.0f;
}
void BreakoutGame::moveRight() {
    padX = min(W - PAD_W, padX + 7);
    if (!launched) bx = padX + PAD_W / 2.0f;
}
void BreakoutGame::launch() { if (!launched) launched = true; }

bool BreakoutGame::update() {
    if (isGameOver() || isWon() || !launched) return false;
    unsigned long now = millis();
    if (now - lastMs < 16) return false;
    lastMs = now;

    bx += vx; by += vy;

    // Walls
    if (bx <= BALL_R)       { bx = BALL_R;       vx =  fabsf(vx); }
    if (bx >= W - BALL_R)   { bx = W - BALL_R;   vx = -fabsf(vx); }
    if (by <= BALL_R)       { by = BALL_R;        vy =  fabsf(vy); }

    // Paddle bounce
    if (vy > 0 && by + BALL_R >= PAD_Y &&
        bx >= padX - BALL_R && bx <= padX + PAD_W + BALL_R) {
        float rel = (bx - (padX + PAD_W / 2.0f)) / (float)(PAD_W / 2);
        vx = rel * 3.8f;
        vy = -fabsf(vy);
        by = PAD_Y - BALL_R;
    }

    // Fell off bottom
    if (by > H + 10) {
        lives--;
        resetBall();
        return true;
    }

    // Bricks
    for (int r = 0; r < ROWS; r++) {
        int brickY = BRICK_OFY + r * BRICK_H;
        for (int c = 0; c < COLS; c++) {
            if (!bricks[r][c]) continue;
            int brickX = c * BRICK_W;
            if (bx + BALL_R > brickX && bx - BALL_R < brickX + BRICK_W &&
                by + BALL_R > brickY && by - BALL_R < brickY + BRICK_H) {
                bricks[r][c] = false;
                bricksLeft--;
                score += (ROWS - r) * 10;
                float ovX = min(bx + BALL_R - brickX, brickX + BRICK_W - (bx - BALL_R));
                float ovY = min(by + BALL_R - brickY, brickY + BRICK_H - (by - BALL_R));
                if (ovX < ovY) vx = -vx; else vy = -vy;
                return true;
            }
        }
    }
    return true;
}
