#pragma once
#include <Arduino.h>
#include <Adafruit_GC9A01A.h>

class SnakeGame {
public:
    SnakeGame();

    void reset();
    void setDirection(int dx, int dy);
    void update(Adafruit_GC9A01A& tft);
    void render(Adafruit_GC9A01A& tft) const;
    bool isGameOver() const { return gameOver; }
    int getScore() const { return score; }

private:
    static const int COLS = 22;
    static const int ROWS = 16;
    static const int CELL = 10;
    static const int OFSX = 10;
    static const int OFSY = 25;
    static const int MAX_LEN = 100;

    struct Point { int x, y; };
    Point body[MAX_LEN];
    int length;
    int dirX, dirY;
    int nextDirX, nextDirY;
    int foodX, foodY;
    int score;
    bool gameOver;
    unsigned long lastMoveMs;
    int moveDelay;

    void spawnFood();
    bool checkCollision(int x, int y) const;
    void drawCell(Adafruit_GC9A01A& tft, int x, int y, uint16_t color) const;
};
