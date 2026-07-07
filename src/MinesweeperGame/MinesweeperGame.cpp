#include "MinesweeperGame.h"
#include <stdlib.h>

MinesweeperGame::MinesweeperGame() { reset(); }

void MinesweeperGame::reset() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) {
            mine[r][c] = revealed[r][c] = flagged[r][c] = false;
            adj[r][c] = 0;
        }
    cx = COLS / 2; cy = ROWS / 2;
    flagCount = 0;
    hiddenSafe = ROWS * COLS - MINES;
    firstReveal = true;
    state = PLAYING;
}

void MinesweeperGame::moveCursor(int dx, int dy) {
    cx = constrain(cx + dx, 0, COLS - 1);
    cy = constrain(cy + dy, 0, ROWS - 1);
}

void MinesweeperGame::placeMines(int avoidR, int avoidC) {
    int placed = 0;
    while (placed < MINES) {
        int r = rand() % ROWS;
        int c = rand() % COLS;
        if (mine[r][c]) continue;
        // Avoid 3x3 around first click
        if (abs(r - avoidR) <= 1 && abs(c - avoidC) <= 1) continue;
        mine[r][c] = true;
        placed++;
    }
}

void MinesweeperGame::calcAdj() {
    static const int DR[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    static const int DC[] = {-1, 0, 1,-1, 1,-1, 0, 1};
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++) {
            int cnt = 0;
            for (int d = 0; d < 8; d++) {
                int nr = r + DR[d], nc = c + DC[d];
                if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && mine[nr][nc]) cnt++;
            }
            adj[r][c] = cnt;
        }
}

void MinesweeperGame::floodReveal(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;
    if (revealed[r][c] || flagged[r][c] || mine[r][c]) return;
    revealed[r][c] = true;
    hiddenSafe--;
    if (adj[r][c] == 0) {
        static const int DR[] = {-1,-1,-1, 0, 0, 1, 1, 1};
        static const int DC[] = {-1, 0, 1,-1, 1,-1, 0, 1};
        for (int d = 0; d < 8; d++) floodReveal(r + DR[d], c + DC[d]);
    }
}

void MinesweeperGame::reveal() {
    if (state != PLAYING) return;
    int r = cy, c = cx;
    if (revealed[r][c] || flagged[r][c]) return;

    if (firstReveal) {
        firstReveal = false;
        placeMines(r, c);
        calcAdj();
    }

    if (mine[r][c]) {
        // Reveal all mines
        for (int rr = 0; rr < ROWS; rr++)
            for (int cc = 0; cc < COLS; cc++)
                if (mine[rr][cc]) revealed[rr][cc] = true;
        state = LOST;
        return;
    }

    floodReveal(r, c);
    if (hiddenSafe <= 0) state = WON;
}

void MinesweeperGame::toggleFlag() {
    if (state != PLAYING) return;
    int r = cy, c = cx;
    if (revealed[r][c]) return;
    if (flagged[r][c]) {
        flagged[r][c] = false;
        flagCount--;
    } else if (flagCount < MINES) {
        flagged[r][c] = true;
        flagCount++;
    }
}
