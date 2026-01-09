#include "Enemy.h"
#include "Player.h"
#include <cmath>
#include <random>

Enemy::Enemy()
{
    x = 0;
    y = 0;
    speed = 0;
    angle = 0;
    active = false;    // all enemies start inactive
    type = EnemyType::Base;
}

float Enemy::distanceTo(const Player& player) const {
    float dx = player.x - x;
    float dy = player.y - y;
    return std::sqrt(dx*dx + dy*dy);
}

void Enemy::activate(int tx, int ty, EnemyType t)
{
    x = float(tx) + 0.5f;
    y = float(ty) + 0.5f;
    type = t;
    active = true;

    // assign a small random lateral offset to separate enemies visually
    lateralOffset = ((rand() % 201) - 100) / 100.0f; // -1.0 to 1.0 units

    // basic stats per type
    switch (type) {
        case EnemyType::Base:
            speed = 1.5f;
            break;
        case EnemyType::Fast:
            speed = 2.8f;
            break;
        case EnemyType::Tank:
            speed = 1.0f;
            break;
    }
}

bool Enemy::hasLineOfSight(const Player& player, const Map& map) const
{
    float dx = player.x - x;
    float dy = player.y - y;

    float dist = std::sqrt(dx*dx + dy*dy);

    float step = 0.1f; // small step for accuracy
    float steps = dist / step;

    float sx = x;
    float sy = y;

    float incX = dx / steps;
    float incY = dy / steps;

    for (int i = 0; i < (int)steps; i++) {
        sx += incX;
        sy += incY;

        int mx = int(sx);
        int my = int(sy);

        // hit a wall
        if (map.get(mx,my).type == Map::TileType::Wall) {
            return false;
        }
    }

    return true;
}

void Enemy::chasePlayer(float dt, const Player& player)
{
    float dx = player.x - x;
    float dy = player.y - y;
    angle = std::atan2(dy, dx);
    x += std::cos(angle) * speed * dt;
    y += std::sin(angle) * speed * dt;
}

void Enemy::wander(float dt)
{
    static float changeTimer = 0.0f;

    changeTimer -= dt;

    if (changeTimer <= 0.0f) {
        // pick a new random movement direction
        wanderAngle = ((std::rand() % 628) / 100.0f) - 3.14f;
        changeTimer = 2.0f + (std::rand() % 200) / 100.0f; // 2–4 seconds
    }

    x += std::cos(wanderAngle) * speed * 0.3f * dt; // slower movement
    y += std::sin(wanderAngle) * speed * 0.3f * dt;
}

void Enemy::update(float dt, const Player& player, const Map& map)
{
    if (!active) return;

    bool seesPlayer = hasLineOfSight(player, map);

    switch (state)
    {
        case EnemyState::Idle:
            if (seesPlayer) {
                state = EnemyState::Chasing;
            } else {
                wander(dt);   // small random movement
            }
            break;

        case EnemyState::Chasing:
            if (seesPlayer) {
                chasePlayer(dt, player);
                loseSightTimer = 1.0f; // 1 second delay after losing LOS
            } else {
                state = EnemyState::Searching;
            }
            break;

        case EnemyState::Searching:
            loseSightTimer -= dt;
            chasePlayer(dt, player);   // keep chasing briefly

            if (loseSightTimer <= 0.0f) {
                state = EnemyState::Idle;
            }
            break;
    }
}
