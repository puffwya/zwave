#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include "Map.h"
#include <SDL2/SDL.h>

enum class EnemyType {
    Base,
    Fast,
    Tank
};

enum class EnemyState {
    Idle,
    Chasing,
    Searching
};

enum class EnemyAnimState {
    Idle,
    Walk,
    Attack
};

class Player;

class Enemy {
public:
    float x = 0, y = 0, z = 0;
    float height = 0;
    float speed = 0;
    float angle = 0;
    bool active = false;
    EnemyType type = EnemyType::Base;

    EnemyAnimState animState = EnemyAnimState::Idle;
    int animFrame = 0;
    float animTimer = 0.0f;

    bool attacking = false;

    int spriteW = 0;
    int spriteH = 0;
    std::vector<uint32_t> spritePixels;

    EnemyState state = EnemyState::Idle;
    float loseSightTimer = 0.0f;
    float wanderAngle = 0.0f;
    float wanderTimer = 0.0f;

    float lateralOffset = 0.0f;

    Enemy();

    void activate(int tx, int ty, EnemyType t);

    bool hasLineOfSight(const Player& player, const Map& map) const;
    void chasePlayer(float dt, const Player& player);
    void wander(float dt);
    void update(float dt, const Player& player, const Map& map);

    void updateAnimation(float dt);

    void deactivate() { active = false; }

    float distanceTo(const Player& player) const;
};

#endif

