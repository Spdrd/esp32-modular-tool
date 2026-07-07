#pragma once
#include <Arduino.h>

class FlappyGame {
public:
    static const int W      = 210;
    static const int H      = 195;
    static const int PIPE_W = 18;
    static const int GAP    = 52;
    static const int BIRD_X = 38;
    static const int BIRD_R = 5;

    FlappyGame();
    void reset();
    bool update();
    void flap();

    int  getBirdY()   const { return (int)birdY; }
    int  getPipeX()   const { return pipeX; }
    int  getGapY()    const { return gapY; }
    int  getScore()   const { return score; }
    bool isGameOver() const { return gameOver; }
    bool isStarted()  const { return started; }

private:
    float birdY, velY;
    int   pipeX, gapY;
    int   score;
    bool  gameOver, started;
    unsigned long lastMs;

    void nextPipe();
};
