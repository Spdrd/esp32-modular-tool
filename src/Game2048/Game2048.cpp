#include "Game2048.h"

Game2048::Game2048() {
    reset();
}

void Game2048::reset() {
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            grid[r][c] = 0;
    score    = 0;
    won      = false;
    gameOver = false;
    addRandom();
    addRandom();
}

void Game2048::addRandom() {
    // Collect empty positions
    int emptyRows[16];
    int emptyCols[16];
    int count = 0;

    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (grid[r][c] == 0) {
                emptyRows[count] = r;
                emptyCols[count] = c;
                count++;
            }
        }
    }

    if (count == 0) return;

    int idx = random(count);
    int r   = emptyRows[idx];
    int c   = emptyCols[idx];

    // 90% chance of 2, 10% chance of 4
    grid[r][c] = (random(10) < 9) ? 2 : 4;
}

// Slides a single row to the left in-place.
// Returns true if anything changed.
bool Game2048::slideRowLeft(int row[4], int& addedScore) {
    int original[4];
    for (int i = 0; i < 4; i++) original[i] = row[i];

    // Step 1: compact (shift non-zeros left)
    int tmp[4] = {0, 0, 0, 0};
    int pos = 0;
    for (int i = 0; i < 4; i++) {
        if (row[i] != 0) {
            tmp[pos++] = row[i];
        }
    }
    for (int i = 0; i < 4; i++) row[i] = tmp[i];

    // Step 2: merge adjacent equal tiles
    for (int i = 0; i < 3; i++) {
        if (row[i] != 0 && row[i] == row[i + 1]) {
            row[i] *= 2;
            addedScore += row[i];
            if (row[i] == 2048) won = true;
            row[i + 1] = 0;
            i++; // skip merged tile
        }
    }

    // Step 3: compact again after merges
    int tmp2[4] = {0, 0, 0, 0};
    pos = 0;
    for (int i = 0; i < 4; i++) {
        if (row[i] != 0) {
            tmp2[pos++] = row[i];
        }
    }
    for (int i = 0; i < 4; i++) row[i] = tmp2[i];

    // Check if anything changed
    for (int i = 0; i < 4; i++) {
        if (row[i] != original[i]) return true;
    }
    return false;
}

void Game2048::transpose() {
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            int tmp       = grid[i][j];
            grid[i][j]    = grid[j][i];
            grid[j][i]    = tmp;
        }
    }
}

void Game2048::reverseRows() {
    for (int r = 0; r < 4; r++) {
        int tmp      = grid[r][0];
        grid[r][0]   = grid[r][3];
        grid[r][3]   = tmp;

        tmp          = grid[r][1];
        grid[r][1]   = grid[r][2];
        grid[r][2]   = tmp;
    }
}

void Game2048::checkGameOver() {
    if (!canMove()) gameOver = true;
}

// Core helper: applies optional transpose/reverse, slides all rows left,
// reverts the transforms, and if anything moved calls addRandom + checkGameOver.
bool Game2048::applySlide(bool doTranspose, bool doReverse) {
    if (doTranspose) transpose();
    if (doReverse)   reverseRows();

    bool changed = false;
    for (int r = 0; r < 4; r++) {
        int added = 0;
        if (slideRowLeft(grid[r], added)) {
            changed = true;
            score += added;
        }
    }

    if (doReverse)   reverseRows();
    if (doTranspose) transpose();

    if (changed) {
        addRandom();
        checkGameOver();
    }
    return changed;
}

bool Game2048::slideLeft() {
    return applySlide(false, false);
}

bool Game2048::slideRight() {
    return applySlide(false, true);
}

bool Game2048::slideUp() {
    return applySlide(true, false);
}

bool Game2048::slideDown() {
    return applySlide(true, true);
}

bool Game2048::canMove() const {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (grid[r][c] == 0) return true;

            // Check right neighbor
            if (c < 3 && grid[r][c] == grid[r][c + 1]) return true;

            // Check bottom neighbor
            if (r < 3 && grid[r][c] == grid[r + 1][c]) return true;
        }
    }
    return false;
}
