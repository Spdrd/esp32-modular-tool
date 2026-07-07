#pragma once
#include <Arduino.h>

class PongGame {
public:
    static const int W         = 200;
    static const int H         = 150;
    static const int PAD_H     = 24;
    static const int PAD_W     = 4;
    static const int BALL_S    = 4;
    static const int WIN_SCORE = 5;

    PongGame();
    void reset();
    bool update();
    void moveUp();
    void moveDown();

    int  getBallX()       const { return (int)bx; }
    int  getBallY()       const { return (int)by; }
    int  getPlayerY()     const { return playerY; }
    int  getAiY()         const { return aiY; }
    int  getPlayerScore() const { return pScore; }
    int  getAiScore()     const { return aScore; }
    bool isGameOver()     const { return gameOver; }
    bool playerWon()      const { return pScore >= WIN_SCORE; }

private:
    float bx, by, vx, vy;
    int   playerY, aiY;
    int   pScore, aScore;
    bool  gameOver;
    unsigned long lastMs;

    void resetBall(int dir);
};
