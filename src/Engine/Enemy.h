#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include "Map.h"
#include "../audio/AudioManager.h"
#include <SDL2/SDL.h>

enum class EnemyType {
    Base,
    Fast,
    Tank,
    Shooter
};

enum class EnemyState {
    Idle,
    Chasing,
    Searching,
    Attacking
};

enum class EnemyAnimState {
    Idle,
    Walk,
    Attack,
    Death
};

class Player;

class EnemyManager;

struct Animation;

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

    // Attack data
    float attackRange = 0.0f;
    int attackDamage = 0;
    float attackCooldown = 0.0f;
    float attackTimer = 0.0f;

    bool hasDealtDamageThisAttack = false;
    int attackHitFrame = 0; // animation frame where damage occurs

    int health = 0;
    int maxHealth = 0;

    int spriteW = 0;
    int spriteH = 0;

    std::vector<uint32_t> spritePixels;

    EnemyManager* managerPtr = nullptr;
    
    Animation* normalAnimation  = nullptr;
    Animation* damagedAnimation = nullptr;

    EnemyState state = EnemyState::Idle;
    float loseSightTimer = 0.0f;
    float wanderAngle = 0.0f;
    float wanderTimer = 0.0f;

    float lateralOffset = 0.0f;

    Enemy();

    void activate(int tx, int ty, EnemyType t, EnemyManager& manager);

    void takeDamage(int amount);
    bool isDead() const;

    bool hasLineOfSight(const Player& player, const Map& map) const;
    void chasePlayer(float dt, const Player& player);
    void wander(float dt);
    void update(float dt, const Player& player, const Map& map, AudioManager& audio);

    bool canAttack(const Player& player) const;

    void handleAttack(float dt, Player& player, AudioManager& audio);

    bool isDamaged() const;

    bool deathAnimFinished = false;

    bool deathJustFinished = false;

    void updateAnimation(float dt);

    void deactivate() { active = false; }

    void reset();

    float distanceTo(const Player& player) const;

private:
    int getMaxHealthForType(EnemyType t) const;

    float getHitChance(const Player& player) const;

    float getShieldMultiplier() const;
};

#endif

