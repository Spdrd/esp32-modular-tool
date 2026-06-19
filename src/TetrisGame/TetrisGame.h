#pragma once
#include <Arduino.h>

class TetrisGame {
public:
    static const int COLS = 10;
    static const int ROWS = 20;

    TetrisGame();
    void reset();
    bool update();       // retorna true si hay que redibujar

    void moveLeft();
    void moveRight();
    void rotate();
    void rotateCCW();
    void softDrop();
    void hardDrop();

    bool isGameOver()    const { return gameOver; }
    int  getScore()      const { return score; }
    int  getLevel()      const { return level; }
    int  getLinesTotal() const { return linesTotal; }
    int  getNextPiece()  const { return nextPiece; }

    // Retorna grid con pieza actual + ghost fusionados (0=vacío, 1-7=colores, 8=ghost)
    void getDisplayGrid(uint8_t out[ROWS][COLS]) const;
    // Retorna las 4 celdas (row,col) de la pieza nextPiece en rotación 0
    void getNextCells(int8_t out[4][2]) const;

private:
    uint8_t board[ROWS][COLS];
    int pieceType, pieceRot, pieceX, pieceY;
    int nextPiece;
    int score, level, linesTotal;
    bool gameOver;
    bool dirty;
    unsigned long lastDropMs;

    void spawnPiece();
    bool canPlace(int type, int rot, int px, int py) const;
    void lockPiece();
    int  clearLines();
    int  getGhostY() const;
    int  dropInterval() const;
};
