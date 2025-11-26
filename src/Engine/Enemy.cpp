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
    sprite = nullptr;
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

void Enemy::update(float dt, const Player& player)
{
    if (!active) return;

    // Vector from enemy to player
    float dx = player.x - x;
    float dy = player.y - y;

    // Distance to player (avoid division by zero)
    float dist = std::sqrt(dx*dx + dy*dy);
    if (dist == 0.0f) return;

    // Normalize direction to player
    float dirX = dx / dist;
    float dirY = dy / dist;

    // Perpendicular vector for lateral offset
    float perpX = -dirY;
    float perpY = dirX;

    // Target position adjusted by lateral offset
    float targetX = player.x + perpX * lateralOffset;
    float targetY = player.y + perpY * lateralOffset;

    // Compute movement direction toward adjusted target
    float moveX = targetX - x;
    float moveY = targetY - y;
    float moveDist = std::sqrt(moveX*moveX + moveY*moveY);
    if (moveDist == 0.0f) return;

    float moveDirX = moveX / moveDist;
    float moveDirY = moveY / moveDist;

    // Update enemy angle for rendering (optional)
    angle = std::atan2(moveDirY, moveDirX);

    // Move enemy
    x += moveDirX * speed * dt;
    y += moveDirY * speed * dt;
}

