#pragma once
#include <Arduino.h>

class Game2048 {
public:
    Game2048();
    void reset();

    bool slideUp();
    bool slideDown();
    bool slideLeft();
    bool slideRight();

    int  getCell(int row, int col) const { return grid[row][col]; }
    int  getScore()    const { return score; }
    bool isWon()       const { return won; }
    bool isGameOver()  const { return gameOver; }

private:
    int  grid[4][4];
    int  score;
    bool won;
    bool gameOver;

    void addRandom();
    bool canMove() const;
    bool slideRowLeft(int row[4], int& addedScore);
    void transpose();
    void reverseRows();
    bool applySlide(bool doTranspose, bool doReverse);
    void checkGameOver();
};
