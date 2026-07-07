#include "DoomGame.h"
#include <math.h>

const uint8_t DoomGame::MAP[DoomGame::MAP_H][DoomGame::MAP_W] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,1,0,0,1},
    {1,0,1,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,1,1,0,0,0,1},
    {1,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1},
};

DoomGame::DoomGame() { reset(); }

void DoomGame::reset() {
    px = 1.5f; py = 1.5f; angle = 0.0f;
    health = 100; ammo = 25; kills = 0; shootTimer = 0; lastUpdateMs = 0;

    static const float ex[] = {7.5f, 4.5f, 7.5f, 4.5f, 6.5f};
    static const float ey[] = {7.5f, 3.5f, 3.5f, 6.5f, 5.5f};
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i] = {ex[i], ey[i], 3, true};
}

bool DoomGame::isWall(float x, float y) const {
    int mx = (int)x, my = (int)y;
    if (mx < 0 || mx >= MAP_W || my < 0 || my >= MAP_H) return true;
    return MAP[my][mx] == 1;
}

void DoomGame::turnLeft()     { angle -= 0.10f; }
void DoomGame::turnRight()    { angle += 0.10f; }

void DoomGame::moveForward() {
    float nx = px + cosf(angle) * 0.14f;
    float ny = py + sinf(angle) * 0.14f;
    if (!isWall(nx, py)) px = nx;
    if (!isWall(px, ny)) py = ny;
}

void DoomGame::moveBackward() {
    float nx = px - cosf(angle) * 0.14f;
    float ny = py - sinf(angle) * 0.14f;
    if (!isWall(nx, py)) px = nx;
    if (!isWall(px, ny)) py = ny;
}

void DoomGame::shoot() {
    if (ammo <= 0 || shootTimer > 0) return;
    ammo--;
    shootTimer = 8;

    float bestDist = 8.0f;
    int bestIdx = -1;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        float dx = enemies[i].x - px;
        float dy = enemies[i].y - py;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist > bestDist) continue;
        float toEnemy = atan2f(dy, dx);
        float diff = toEnemy - angle;
        while (diff >  (float)M_PI) diff -= 2.0f * (float)M_PI;
        while (diff < -(float)M_PI) diff += 2.0f * (float)M_PI;
        if (fabsf(diff) < 0.22f) { bestDist = dist; bestIdx = i; }
    }
    if (bestIdx >= 0) {
        enemies[bestIdx].hp--;
        if (enemies[bestIdx].hp <= 0) {
            enemies[bestIdx].alive = false;
            kills++;
        }
    }
}

bool DoomGame::update() {
    if (isGameOver()) return false;
    unsigned long now = millis();
    if (now - lastUpdateMs < 50) return false; // enemies tick at 20 Hz
    lastUpdateMs = now;

    if (shootTimer > 0) shootTimer--;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) continue;
        float dx = px - enemies[i].x;
        float dy = py - enemies[i].y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 0.55f) {
            health -= 4;
            if (health < 0) health = 0;
        } else if (dist < 4.0f) {
            float spd = 0.04f / (dist * 0.5f + 0.5f);
            float nx = enemies[i].x + (dx / dist) * spd;
            float ny = enemies[i].y + (dy / dist) * spd;
            if (!isWall(nx, enemies[i].y)) enemies[i].x = nx;
            if (!isWall(enemies[i].x, ny)) enemies[i].y = ny;
        }
    }
    return true;
}
