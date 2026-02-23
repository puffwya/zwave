#ifndef ENEMY_H
#define ENEMY_H

#include <vector>
#include <SDL2/SDL.h>
#include "Map.h"
#include "../audio/AudioManager.h"

class Player;
class EnemyManager;
struct Animation;

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

class Enemy {
public:
    // Core
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float height = 0;

    float speed = 0.0f;
    float angle = 0.0f;
    bool active = false;

    EnemyType type = EnemyType::Base;
    EnemyState state = EnemyState::Idle;
    EnemyAnimState animState = EnemyAnimState::Idle;

    // Health
    int health = 0;
    int maxHealth = 0;

    // Combat
    float attackRange = 0.0f;
    int attackDamage = 0;
    float attackCooldown = 0.0f;
    float attackTimer = 0.0f;
    int attackHitFrame = 0;
    bool attacking = false;
    bool hasDealtDamageThisAttack = false;

    // Animation
    int animFrame = 0;
    float animTimer = 0.0f;
    bool deathAnimFinished = false;
    bool deathJustFinished = false;

    Animation* normalAnimation  = nullptr;
    Animation* damagedAnimation = nullptr;

    int spriteW = 0;
    int spriteH = 0;
    std::vector<uint32_t> spritePixels;

    // Enemy AI
    float ambientSoundTimer = 2.0f + ((float)rand() / RAND_MAX) * 10.0f;
    float loseSightTimer = 0.0f;
    float wanderAngle = 0.0f;
    float wanderTimer = 0.0f;
    float lateralOffset = 0.0f;

    EnemyManager* managerPtr = nullptr;

    Enemy();

    void activate(int tx, int ty, EnemyType t, EnemyManager& manager);
    void update(float dt, const Player& player, const Map& map, AudioManager& audio, EnemyType t);
    void reset();
    void deactivate() { active = false; }

    void takeDamage(int amount);
    bool isDead() const;
    bool isDamaged() const;

    bool hasLineOfSight(const Player& player, const Map& map) const;
    void chasePlayer(float dt, const Player& player, AudioManager& audio);
    void wander(float dt, AudioManager& audio);

    bool canAttack(const Player& player) const;
    void handleAttack(float dt, Player& player, AudioManager& audio);

    void updateAnimation(float dt);
    float distanceTo(const Player& player) const;

private:
    int getMaxHealthForType(EnemyType t) const;
    float getHitChance(const Player& player) const;
    float getShieldMultiplier() const;

    void playWanderSound(AudioManager& audio);
    void playChaseSound(AudioManager& audio);
};

#endif
