#pragma once
#include <Arduino.h>

class BreakoutGame {
public:
    static const int COLS    = 7;
    static const int ROWS    = 5;
    static const int BRICK_W = 26;
    static const int BRICK_H = 10;
    static const int W       = COLS * BRICK_W;   // 182
    static const int H       = 175;
    static const int PAD_W   = 32;
    static const int PAD_Y   = 168;

    BreakoutGame();
    void reset();
    bool update();
    void moveLeft();
    void moveRight();
    void launch();

    const bool (*getBricks() const)[COLS] { return bricks; }
    int  getPaddleX()           const { return padX; }
    int  getBallX()             const { return (int)bx; }
    int  getBallY()             const { return (int)by; }
    bool getBrick(int r, int c) const { return bricks[r][c]; }
    int  getBrickColor(int r)   const;
    int  getScore()             const { return score; }
    int  getLives()             const { return lives; }
    bool isLaunched()           const { return launched; }
    bool isGameOver()           const { return lives <= 0; }
    bool isWon()                const { return bricksLeft == 0; }

private:
    static const int BALL_R  = 3;
    static const int BRICK_OFY = 14;

    bool  bricks[ROWS][COLS];
    int   bricksLeft;
    float bx, by, vx, vy;
    int   padX;
    int   score, lives;
    bool  launched;
    unsigned long lastMs;

    void resetBall();
};
