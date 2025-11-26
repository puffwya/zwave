#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include <SDL2/SDL.h>

enum class EnemyType {
    Base,
    Fast,
    Tank
};

class Player;

class Enemy {
public:
    float x, y;
    float speed;
    float angle;
    bool active;
    EnemyType type;

    SDL_Texture* sprite;

    Enemy();
    // Initialize an enemy at a tile
    void activate(int tx, int ty, EnemyType t);

    // AI decision-making
    void update(float dt, const Player& player);

    // Deactivate enemy for memory-pool reuse
    void deactivate() { active = false; }
};

#endif

