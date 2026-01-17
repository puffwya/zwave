#ifndef ENEMY_H
#define ENEMY_H

#include <string>
#include "Map.h"
#include "Texture.h"
#include <SDL2/SDL.h>

enum class EnemyType {
    Base,
    Fast,
    Tank
};

enum class EnemyState {
    Idle,
    Chasing,
    Searching // continues chasing briefly after losing sight
};

enum class EnemyAnimState {
    Idle,
    Walk,
    Attack
};

class Player;

class Enemy {
public:
    float x, y, z;
    float height;
    float speed;
    float angle;
    bool active;
    EnemyType type;

    EnemyAnimState animState = EnemyAnimState::Idle;
    int animFrame = 0;
    float animTimer = 0.0f;

    void updateAnimation(float dt);

    bool attacking = false;

    // CPU-side sprite
    int spriteW = 0;
    int spriteH = 0;
    std::vector<uint32_t> spritePixels;

    // Enemy AI states
    EnemyState state = EnemyState::Idle; // Start Idle by default 
    float loseSightTimer = 0.0f;   // countdown after losing LOS
    float wanderAngle = 0.0f;
    float wanderTimer = 0.0f;

    float lateralOffset = 0.0f;  // persistent offset for chasing

    Enemy();
    // Initialize an enemy at a tile
    void activate(int tx, int ty, EnemyType t);

    // enemy movement state functions
    bool hasLineOfSight(const Player& player, const Map& map) const;
    void chasePlayer(float dt, const Player& player);
    void wander(float dt);
    void update(float dt, const Player& player, const Map& map);

    // AI decision-making
    void update(float dt, const Player& player);

    // Deactivate enemy for memory-pool reuse
    void deactivate() { active = false; }

    float distanceTo(const Player& player) const;

};

#endif

