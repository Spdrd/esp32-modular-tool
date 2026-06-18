#pragma once
#include <Arduino.h>

class SnakeGame {
public:
    SnakeGame();

    void reset();
    void setDirection(int dx, int dy);
    bool update();

    bool isGameOver() const { return gameOver; }
    int getScore() const { return score; }
    int getLength() const { return length; }
    int getBodyX(int i) const { return body[i].x; }
    int getBodyY(int i) const { return body[i].y; }
    int getFoodX() const { return foodX; }
    int getFoodY() const { return foodY; }

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
};
