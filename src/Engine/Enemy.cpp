#include "Enemy.h"
#include "Player.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy() {}

float Enemy::distanceTo(const Player& player) const {
    float dx = player.x - x;
    float dy = player.y - y;
    return std::sqrt(dx*dx + dy*dy);
}

void Enemy::activate(int tx, int ty, EnemyType t) {
    x = float(tx) + 0.5f;
    y = float(ty) + 0.5f;
    type = t;
    active = true;

    lateralOffset = ((rand() % 201) - 100) / 100.0f; // -1.0 to 1.0

    switch (type) {
        case EnemyType::Base: speed = 1.5f; break;
        case EnemyType::Fast: speed = 2.8f; break;
        case EnemyType::Tank: speed = 1.0f; break;
    }
}

bool Enemy::hasLineOfSight(const Player& player, const Map& map) const {
    float dx = player.x - x;
    float dy = player.y - y;
    float dist = std::sqrt(dx*dx + dy*dy);

    float step = 0.1f;
    int steps = int(dist / step);
    float sx = x, sy = y;
    float incX = dx / steps;
    float incY = dy / steps;

    for (int i = 0; i < steps; i++) {
        sx += incX;
        sy += incY;
        int mx = int(sx);
        int my = int(sy);
        if (map.get(mx,my).type == Map::TileType::Wall) return false;
    }

    return true;
}

void Enemy::chasePlayer(float dt, const Player& player) {
    float dx = player.x - x;
    float dy = player.y - y;
    angle = std::atan2(dy, dx);
    x += std::cos(angle) * speed * dt;
    y += std::sin(angle) * speed * dt;
}

void Enemy::wander(float dt) {
    static float changeTimer = 0.0f;
    changeTimer -= dt;

    if (changeTimer <= 0.0f) {
        wanderAngle = ((rand() % 628) / 100.0f) - 3.14f;
        changeTimer = 2.0f + (rand() % 200) / 100.0f;
    }

    x += std::cos(wanderAngle) * speed * 0.3f * dt;
    y += std::sin(wanderAngle) * speed * 0.3f * dt;
}

void Enemy::updateAnimation(float dt) {
    animTimer += dt;

    float frameTime = 0.15f;
    if (animState == EnemyAnimState::Idle) frameTime = 0.4f;
    else if (animState == EnemyAnimState::Walk) frameTime = 0.12f;

    if (animTimer >= frameTime) {
        animTimer -= frameTime;
        animFrame++;
    }
}

void Enemy::update(float dt, const Player& player, const Map& map) {
    if (!active) return;

    bool seesPlayer = hasLineOfSight(player, map);

    switch (state) {
        case EnemyState::Idle:
            animState = EnemyAnimState::Idle;
            if (seesPlayer) state = EnemyState::Chasing;
            else wander(dt);
            break;

        case EnemyState::Chasing:
            animState = EnemyAnimState::Walk;
            if (seesPlayer) {
                chasePlayer(dt, player);
                loseSightTimer = 1.0f;
            } else state = EnemyState::Searching;
            break;

        case EnemyState::Searching:
            animState = EnemyAnimState::Walk;
            loseSightTimer -= dt;
            chasePlayer(dt, player);
            if (loseSightTimer <= 0.0f) state = EnemyState::Idle;
            break;
    }

    updateAnimation(dt);
}

