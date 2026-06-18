#include "SnakeGame.h"
#include <stdlib.h>

SnakeGame::SnakeGame() {
    reset();
}

void SnakeGame::reset() {
    length = 3;
    for (int i = 0; i < length; i++) {
        body[i].x = COLS / 2 - i;
        body[i].y = ROWS / 2;
    }
    dirX = 1;  dirY = 0;
    nextDirX = 1; nextDirY = 0;
    score = 0;
    gameOver = false;
    moveDelay = 200;
    lastMoveMs = 0;
    spawnFood();
}

void SnakeGame::setDirection(int dx, int dy) {
    if (dx == -dirX && dy == -dirY) return;
    nextDirX = dx;
    nextDirY = dy;
}

void SnakeGame::spawnFood() {
    bool freeCells[COLS][ROWS];
    for (int x = 0; x < COLS; x++)
        for (int y = 0; y < ROWS; y++)
            freeCells[x][y] = true;

    for (int i = 0; i < length; i++)
        freeCells[body[i].x][body[i].y] = false;

    int count = 0;
    for (int x = 0; x < COLS; x++)
        for (int y = 0; y < ROWS; y++)
            if (freeCells[x][y]) count++;

    if (count == 0) return;

    int r = rand() % count;
    for (int x = 0; x < COLS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if (freeCells[x][y]) {
                if (r == 0) { foodX = x; foodY = y; return; }
                r--;
            }
        }
    }
}

bool SnakeGame::checkCollision(int x, int y) const {
    if (x < 0 || x >= COLS || y < 0 || y >= ROWS) return true;
    for (int i = 0; i < length; i++)
        if (body[i].x == x && body[i].y == y) return true;
    return false;
}

bool SnakeGame::update() {
    unsigned long now = millis();
    if (gameOver) return false;
    if (now - lastMoveMs < (unsigned long)moveDelay) return false;
    lastMoveMs = now;

    dirX = nextDirX;
    dirY = nextDirY;

    int newX = body[0].x + dirX;
    int newY = body[0].y + dirY;

    if (newX < 0 || newX >= COLS || newY < 0 || newY >= ROWS) {
        gameOver = true;
        return true;
    }

    for (int i = 1; i < length; i++) {
        if (body[i].x == newX && body[i].y == newY) {
            gameOver = true;
            return true;
        }
    }

    for (int i = length; i > 0; i--)
        body[i] = body[i - 1];

    body[0].x = newX;
    body[0].y = newY;

    if (newX == foodX && newY == foodY) {
        length++;
        if (length > MAX_LEN) length = MAX_LEN;
        body[length - 1] = body[length - 2];
        score++;
        if (moveDelay > 60) moveDelay -= 5;
        spawnFood();
    }

    return true;
}
