#include "TetrisGame.h"
#include <string.h>

// ---------------------------------------------------------------------------
// PIECES table: [type][rotation][cell][row/col]
// 7 pieces, 4 rotations, 4 cells, 2 coords (row, col)
// ---------------------------------------------------------------------------
static const int8_t PIECES[7][4][4][2] = {
    // 0 = I
    {
        {{1,0},{1,1},{1,2},{1,3}},
        {{0,2},{1,2},{2,2},{3,2}},
        {{2,0},{2,1},{2,2},{2,3}},
        {{0,1},{1,1},{2,1},{3,1}}
    },
    // 1 = O
    {
        {{0,1},{0,2},{1,1},{1,2}},
        {{0,1},{0,2},{1,1},{1,2}},
        {{0,1},{0,2},{1,1},{1,2}},
        {{0,1},{0,2},{1,1},{1,2}}
    },
    // 2 = T
    {
        {{0,1},{1,0},{1,1},{1,2}},
        {{0,1},{1,1},{1,2},{2,1}},
        {{1,0},{1,1},{1,2},{2,1}},
        {{0,1},{1,0},{1,1},{2,1}}
    },
    // 3 = S
    {
        {{0,1},{0,2},{1,0},{1,1}},
        {{0,0},{1,0},{1,1},{2,1}},
        {{0,1},{0,2},{1,0},{1,1}},
        {{0,0},{1,0},{1,1},{2,1}}
    },
    // 4 = Z
    {
        {{0,0},{0,1},{1,1},{1,2}},
        {{0,1},{1,0},{1,1},{2,0}},
        {{0,0},{0,1},{1,1},{1,2}},
        {{0,1},{1,0},{1,1},{2,0}}
    },
    // 5 = J
    {
        {{0,0},{1,0},{1,1},{1,2}},
        {{0,0},{0,1},{1,0},{2,0}},
        {{0,0},{0,1},{0,2},{1,2}},
        {{0,1},{1,1},{2,0},{2,1}}
    },
    // 6 = L
    {
        {{0,2},{1,0},{1,1},{1,2}},
        {{0,0},{1,0},{2,0},{2,1}},
        {{0,0},{0,1},{0,2},{1,0}},
        {{0,0},{0,1},{1,1},{2,1}}
    }
};

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
TetrisGame::TetrisGame() {
    reset();
}

// ---------------------------------------------------------------------------
// reset
// ---------------------------------------------------------------------------
void TetrisGame::reset() {
    memset(board, 0, sizeof(board));
    score       = 0;
    level       = 0;
    linesTotal  = 0;
    gameOver    = false;
    dirty       = true;
    heldPiece   = -1;
    canHoldFlag = true;
    lastDropMs  = millis();
    nextPiece   = random(7);
    spawnPiece();
}

// ---------------------------------------------------------------------------
// spawnPiece
// ---------------------------------------------------------------------------
void TetrisGame::spawnPiece() {
    pieceType   = nextPiece;
    nextPiece   = random(7);
    pieceRot    = 0;
    pieceX      = 3;
    pieceY      = 0;
    canHoldFlag = true;

    if (!canPlace(pieceType, pieceRot, pieceX, pieceY)) {
        gameOver = true;
    }
    dirty = true;
}

// ---------------------------------------------------------------------------
// canPlace
// ---------------------------------------------------------------------------
bool TetrisGame::canPlace(int type, int rot, int px, int py) const {
    for (int i = 0; i < 4; i++) {
        int r   = py + PIECES[type][rot][i][0];
        int col = px + PIECES[type][rot][i][1];

        if (col < 0 || col >= COLS || r >= ROWS) return false;
        if (r >= 0 && board[r][col] != 0)        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// getGhostY
// ---------------------------------------------------------------------------
int TetrisGame::getGhostY() const {
    int gy = pieceY;
    while (canPlace(pieceType, pieceRot, pieceX, gy + 1)) {
        gy++;
    }
    return gy;
}

// ---------------------------------------------------------------------------
// dropInterval  (ms)
// ---------------------------------------------------------------------------
int TetrisGame::dropInterval() const {
    int interval = 800 - level * 80;
    return interval < 80 ? 80 : interval;
}

// ---------------------------------------------------------------------------
// clearLines — returns number of lines cleared
// ---------------------------------------------------------------------------
int TetrisGame::clearLines() {
    int cleared = 0;
    for (int r = ROWS - 1; r >= 0; ) {
        bool full = true;
        for (int c = 0; c < COLS; c++) {
            if (board[r][c] == 0) { full = false; break; }
        }
        if (full) {
            // Shift all rows above down by one
            for (int rr = r; rr > 0; rr--) {
                memcpy(board[rr], board[rr - 1], COLS);
            }
            memset(board[0], 0, COLS);
            cleared++;
            // Do NOT decrement r — recheck same row index
        } else {
            r--;
        }
    }
    return cleared;
}

// ---------------------------------------------------------------------------
// lockPiece
// ---------------------------------------------------------------------------
void TetrisGame::lockPiece() {
    for (int i = 0; i < 4; i++) {
        int r   = pieceY + PIECES[pieceType][pieceRot][i][0];
        int col = pieceX + PIECES[pieceType][pieceRot][i][1];
        if (r >= 0 && r < ROWS && col >= 0 && col < COLS) {
            board[r][col] = (uint8_t)(pieceType + 1);
        }
    }

    int lines = clearLines();
    if (lines > 0) {
        linesTotal += lines;

        // Classic scoring
        static const int linePoints[5] = {0, 100, 300, 500, 800};
        int pts = (lines <= 4) ? linePoints[lines] : linePoints[4];
        score += pts * (level + 1);

        // Level up every 10 lines
        level = linesTotal / 10;
    }

    spawnPiece();
}

// ---------------------------------------------------------------------------
// update — called from game loop
// ---------------------------------------------------------------------------
bool TetrisGame::update() {
    if (gameOver) return dirty;

    unsigned long now = millis();
    if (now - lastDropMs >= (unsigned long)dropInterval()) {
        lastDropMs = now;
        if (canPlace(pieceType, pieceRot, pieceX, pieceY + 1)) {
            pieceY++;
            dirty = true;
        } else {
            lockPiece();
        }
    }

    bool d = dirty;
    dirty   = false;
    return d;
}

// ---------------------------------------------------------------------------
// moveLeft
// ---------------------------------------------------------------------------
void TetrisGame::moveLeft() {
    if (gameOver) return;
    if (canPlace(pieceType, pieceRot, pieceX - 1, pieceY)) {
        pieceX--;
        dirty = true;
    }
}

// ---------------------------------------------------------------------------
// moveRight
// ---------------------------------------------------------------------------
void TetrisGame::moveRight() {
    if (gameOver) return;
    if (canPlace(pieceType, pieceRot, pieceX + 1, pieceY)) {
        pieceX++;
        dirty = true;
    }
}

// ---------------------------------------------------------------------------
// rotate — SRS-style wall kicks (±1, ±2 cols)
// ---------------------------------------------------------------------------
void TetrisGame::rotate() {
    if (gameOver) return;
    int newRot = (pieceRot + 1) % 4;

    // Try original position first, then wall kicks
    static const int kicks[5] = {0, -1, 1, -2, 2};
    for (int k = 0; k < 5; k++) {
        int nx = pieceX + kicks[k];
        if (canPlace(pieceType, newRot, nx, pieceY)) {
            pieceX   = nx;
            pieceRot = newRot;
            dirty    = true;
            return;
        }
    }
    // Rotation failed — do nothing
}

// ---------------------------------------------------------------------------
// rotateCCW
// ---------------------------------------------------------------------------
void TetrisGame::rotateCCW() {
    if (gameOver) return;
    int newRot = (pieceRot + 3) % 4;

    static const int kicks[5] = {0, -1, 1, -2, 2};
    for (int k = 0; k < 5; k++) {
        int nx = pieceX + kicks[k];
        if (canPlace(pieceType, newRot, nx, pieceY)) {
            pieceX   = nx;
            pieceRot = newRot;
            dirty    = true;
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// softDrop
// ---------------------------------------------------------------------------
void TetrisGame::softDrop() {
    if (gameOver) return;
    if (canPlace(pieceType, pieceRot, pieceX, pieceY + 1)) {
        pieceY++;
        score++;
        dirty = true;
    } else {
        lockPiece();
    }
    lastDropMs = millis();
}

// ---------------------------------------------------------------------------
// hardDrop
// ---------------------------------------------------------------------------
void TetrisGame::hardDrop() {
    if (gameOver) return;
    int ghostY = getGhostY();
    int dist   = ghostY - pieceY;
    score     += 2 * dist;
    pieceY     = ghostY;
    lockPiece();
    lastDropMs = millis();
}

// ---------------------------------------------------------------------------
// getDisplayGrid
// ---------------------------------------------------------------------------
void TetrisGame::getDisplayGrid(uint8_t out[ROWS][COLS]) const {
    // Copy board
    memcpy(out, board, sizeof(board));

    // Draw ghost (value 8) — only where the cell is empty
    int ghostY = getGhostY();
    if (ghostY != pieceY) {
        for (int i = 0; i < 4; i++) {
            int r   = ghostY + PIECES[pieceType][pieceRot][i][0];
            int col = pieceX  + PIECES[pieceType][pieceRot][i][1];
            if (r >= 0 && r < ROWS && col >= 0 && col < COLS && out[r][col] == 0) {
                out[r][col] = 8;
            }
        }
    }

    // Draw active piece on top
    for (int i = 0; i < 4; i++) {
        int r   = pieceY + PIECES[pieceType][pieceRot][i][0];
        int col = pieceX  + PIECES[pieceType][pieceRot][i][1];
        if (r >= 0 && r < ROWS && col >= 0 && col < COLS) {
            out[r][col] = (uint8_t)(pieceType + 1);
        }
    }
}

// ---------------------------------------------------------------------------
// getNextCells
// ---------------------------------------------------------------------------
void TetrisGame::getNextCells(int8_t out[4][2]) const {
    for (int i = 0; i < 4; i++) {
        out[i][0] = PIECES[nextPiece][0][i][0];
        out[i][1] = PIECES[nextPiece][0][i][1];
    }
}

// ---------------------------------------------------------------------------
// getHeldCells
// ---------------------------------------------------------------------------
void TetrisGame::getHeldCells(int8_t out[4][2]) const {
    if (heldPiece < 0) {
        for (int i = 0; i < 4; i++) out[i][0] = out[i][1] = -1;
        return;
    }
    for (int i = 0; i < 4; i++) {
        out[i][0] = PIECES[heldPiece][0][i][0];
        out[i][1] = PIECES[heldPiece][0][i][1];
    }
}

// ---------------------------------------------------------------------------
// holdPiece
// ---------------------------------------------------------------------------
void TetrisGame::holdPiece() {
    if (gameOver || !canHoldFlag) return;
    canHoldFlag = false;

    if (heldPiece < 0) {
        // Primera vez: guardar pieza actual y hacer spawn de la siguiente
        heldPiece = pieceType;
        spawnPiece();
        // spawnPiece setea canHoldFlag=true, pero acabamos de usar el hold
        canHoldFlag = false;
    } else {
        // Intercambiar pieza actual con la reservada
        int tmp   = heldPiece;
        heldPiece = pieceType;
        pieceType = tmp;
        pieceRot  = 0;
        pieceX    = 3;
        pieceY    = 0;
        if (!canPlace(pieceType, pieceRot, pieceX, pieceY)) {
            gameOver = true;
        }
    }
    dirty = true;
}
