#include "InvadersGame.h"
#include <stdlib.h>

InvadersGame::InvadersGame() { reset(); }

void InvadersGame::reset() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            inv[r][c] = true;
    invLeft = ROWS * COLS;
    invOfsX = 8; invOfsY = 22; invDirX = 1;
    shipX = W / 2;
    score = 0; gameOver = false;
    bullet    = {0, 0, false};
    invBullet = {0, 0, false};
    lastMs = 0; invShootMs = 0; invMoveTimer = 0;
}

void InvadersGame::moveLeft()  { shipX = max(SHIP_W / 2, shipX - 6); }
void InvadersGame::moveRight() { shipX = min(W - SHIP_W / 2, shipX + 6); }

void InvadersGame::shoot() {
    if (!bullet.active) bullet = {shipX, SHIP_Y - 10, true};
}

bool InvadersGame::update() {
    if (gameOver || isWon()) return false;
    unsigned long now = millis();
    if (now - lastMs < 16) return false;
    lastMs = now;

    // Player bullet
    if (bullet.active) {
        bullet.y -= 5;
        if (bullet.y < 0) {
            bullet.active = false;
        } else {
            for (int r = 0; r < ROWS && bullet.active; r++) {
                for (int c = 0; c < COLS && bullet.active; c++) {
                    if (!inv[r][c]) continue;
                    int ix = invOfsX + c * INV_SPACING;
                    int iy = invOfsY + r * 22;
                    if (abs(bullet.x - ix) < INV_W / 2 + 3 && abs(bullet.y - iy) < INV_H / 2 + 3) {
                        inv[r][c] = false;
                        invLeft--;
                        score += (ROWS - r) * 10;
                        bullet.active = false;
                    }
                }
            }
        }
    }

    // Invader random bullet
    if (!invBullet.active && invLeft > 0 && now - invShootMs > 1100) {
        invShootMs = now;
        // Pick a bottom-most live invader in a random column
        int startCol = rand() % COLS;
        for (int ci = 0; ci < COLS; ci++) {
            int c = (startCol + ci) % COLS;
            for (int r = ROWS - 1; r >= 0; r--) {
                if (inv[r][c]) {
                    invBullet = {invOfsX + c * INV_SPACING, invOfsY + r * 22 + INV_H / 2, true};
                    break;
                }
            }
            if (invBullet.active) break;
        }
    }

    if (invBullet.active) {
        invBullet.y += 3;
        if (invBullet.y > H) {
            invBullet.active = false;
        } else if (abs(invBullet.x - shipX) < SHIP_W / 2 + 3 && abs(invBullet.y - SHIP_Y) < 6) {
            gameOver = true; return true;
        }
    }

    // Invader march
    invMoveTimer++;
    int moveEvery = max(3, 18 - (ROWS * COLS - invLeft));
    if (invMoveTimer >= moveEvery) {
        invMoveTimer = 0;
        invOfsX += invDirX * 4;
        // Check edges
        bool hitEdge = false;
        for (int c = 0; c < COLS && !hitEdge; c++) {
            for (int r = 0; r < ROWS && !hitEdge; r++) {
                if (!inv[r][c]) continue;
                int ix = invOfsX + c * INV_SPACING;
                if (ix < 4 || ix + INV_W > W - 4) hitEdge = true;
            }
        }
        if (hitEdge) {
            invDirX = -invDirX;
            invOfsY += 10;
            if (invOfsY + ROWS * 22 > SHIP_Y - 10) { gameOver = true; return true; }
        }
    }

    return true;
}
