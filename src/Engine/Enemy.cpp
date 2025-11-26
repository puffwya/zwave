#include "Enemy.h"
#include "Player.h"
#include <cmath>

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

void Enemy::activate(int tx, int ty, EnemyType t)
{
    x = float(tx) + 0.5f;
    y = float(ty) + 0.5f;
    type = t;
    active = true;

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

    // simple chase behavior for all enemies (for now)
    float dx = player.x - x;
    float dy = player.y - y;

    angle = std::atan2(dy, dx);

    x += std::cos(angle) * speed * dt;
    y += std::sin(angle) * speed * dt;
}

