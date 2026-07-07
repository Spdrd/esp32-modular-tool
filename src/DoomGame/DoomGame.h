#pragma once
#include <Arduino.h>

class DoomGame {
public:
    static const int MAP_W = 10;
    static const int MAP_H = 10;
    static const int MAX_ENEMIES = 5;

    struct Enemy {
        float x, y;
        int   hp;
        bool  alive;
    };

    DoomGame();
    void reset();
    bool update();

    void turnLeft();
    void turnRight();
    void moveForward();
    void moveBackward();
    void shoot();

    float getPlayerX()    const { return px; }
    float getPlayerY()    const { return py; }
    float getAngle()      const { return angle; }
    int   getHealth()     const { return health; }
    int   getAmmo()       const { return ammo; }
    int   getKills()      const { return kills; }
    bool  isGameOver()    const { return health <= 0; }
    bool  isAllDead()     const { return kills >= MAX_ENEMIES; }
    bool  isShooting()    const { return shootTimer > 0; }

    const Enemy* getEnemies() const { return enemies; }

    // Map: 1=wall, 0=floor
    static const uint8_t MAP[MAP_H][MAP_W];

private:
    float px, py, angle;
    int   health, ammo, kills;
    int   shootTimer;
    unsigned long lastUpdateMs;
    Enemy enemies[MAX_ENEMIES];

    bool isWall(float x, float y) const;
};
