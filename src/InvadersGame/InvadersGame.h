#pragma once
#include <Arduino.h>

class InvadersGame {
public:
    static const int COLS        = 6;
    static const int ROWS        = 3;
    static const int W           = 200;
    static const int H           = 185;
    static const int INV_SPACING = 28;
    static const int INV_W       = 12;
    static const int INV_H       = 8;
    static const int SHIP_W      = 14;
    static const int SHIP_Y      = 170;

    struct Bullet { int x, y; bool active; };

    InvadersGame();
    void reset();
    bool update();
    void moveLeft();
    void moveRight();
    void shoot();

    const bool (*getGrid() const)[COLS] { return inv; }
    int  getShipX()    const { return shipX; }
    int  getInvOfsX()  const { return invOfsX; }
    int  getInvOfsY()  const { return invOfsY; }
    bool getInvader(int r, int c) const { return inv[r][c]; }
    int  getScore()    const { return score; }
    bool isGameOver()  const { return gameOver; }
    bool isWon()       const { return invLeft == 0; }
    const Bullet& getBullet()    const { return bullet; }
    const Bullet& getInvBullet() const { return invBullet; }

private:
    bool inv[ROWS][COLS];
    int  invLeft;
    int  invOfsX, invOfsY, invDirX;
    int  shipX;
    int  score;
    bool gameOver;
    Bullet bullet, invBullet;
    unsigned long lastMs, invShootMs;
    int  invMoveTimer;
};
