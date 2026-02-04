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

void Enemy::activate(int tx, int ty, EnemyType t, EnemyManager& manager) {
    x = float(tx) + 0.5f;
    y = float(ty) + 0.5f;
    type = t;
    active = true;

    lateralOffset = ((rand() % 201) - 100) / 100.0f; // -1.0 to 1.0

    switch (type) {
        case EnemyType::Base: speed = 1.5f; break;
        case EnemyType::Fast: speed = 2.5f; break;
        case EnemyType::Tank: speed = 1.0f; break;
        case EnemyType::Shooter: speed = 1.25f; break;
    }

    switch (type) {
    case EnemyType::Base:
        attackDamage = 20;
        attackRange = 0.8f;
        attackCooldown = 1.2f;
        attackHitFrame = 2;
        break;

    case EnemyType::Fast:
        attackDamage = 30;
        attackRange = 0.7f;
        attackCooldown = 0.6f;
        attackHitFrame = 1;
        break;

    case EnemyType::Tank:
        attackDamage = 75;
        attackRange = 0.9f;
        attackCooldown = 3.0f;
        attackHitFrame = 3;
        break;

    case EnemyType::Shooter:
        attackDamage = 15;
        attackRange = 5.0f;
        attackCooldown = 5.0f;
        attackHitFrame = 2;
        break;
    }

    // Health initialization
    maxHealth = getMaxHealthForType(type);
    health = maxHealth;

    managerPtr = &manager;

    // Sprite assignment
    normalAnimation  = &manager.enemyVisuals.at(type).animations[animState];
    damagedAnimation = &manager.enemyVisualsDamaged.at(type).animations[animState];

    // Set initial spritePixels to normal
    spritePixels = normalAnimation->frames[0].pixels;
    spriteW = normalAnimation->frames[0].w;
    spriteH = normalAnimation->frames[0].h;
    animFrame = 0;
    animTimer = 0.0f;
}

bool Enemy::hasLineOfSight(const Player& player, const Map& map) const {
    constexpr float MAX_LOS_DISTANCE = 10.0f;

    float enemyEyeZ  = 0.5f;
    float playerEyeZ = player.z;

    float heightDiff = playerEyeZ - enemyEyeZ;

    // Too much vertical separation
    if (std::abs(heightDiff) > 0.5f)
        return false;

    float dx = player.x - x;
    float dy = player.y - y;
    float dist = std::sqrt(dx*dx + dy*dy);

    // Distance cap
    if (dist > MAX_LOS_DISTANCE)
        return false;

    float step = 0.1f;
    int steps = int(dist / step);
    if (steps <= 0)
        return true;

    float sx = x;
    float sy = y;
    float incX = dx / steps;
    float incY = dy / steps;

    float blockingHeight = std::max(0.5f, std::abs(heightDiff) * 0.5f);

    for (int i = 0; i < steps; i++) {
        sx += incX;
        sy += incY;

        int mx = int(sx);
        int my = int(sy);

        const auto& tile = map.get(mx, my);

        if (tile.type == Map::TileType::Wall) {
            if (tile.height >= blockingHeight)
                return false;
        }
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

        // Select the correct animation pointer based on current state
        Animation* currentAnim = isDamaged() 
            ? &managerPtr->enemyVisualsDamaged.at(type).animations.at(animState) 
            : &managerPtr->enemyVisuals.at(type).animations.at(animState);

        if (animState == EnemyAnimState::Death) {
            if (animFrame >= currentAnim->frames.size()) {
                animFrame = currentAnim->frames.size() - 1;
                deathAnimFinished = true;
            }
        }
        else {
            if (animFrame >= currentAnim->frames.size())
                animFrame = 0;
        }

        spritePixels = currentAnim->frames[animFrame].pixels;
        spriteW = currentAnim->frames[animFrame].w;
        spriteH = currentAnim->frames[animFrame].h;
    }
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health < 0)
        health = 0;
}

bool Enemy::isDamaged() const {
    return health <= maxHealth / 2;
}

bool Enemy::isDead() const {
    return health <= 0;
}

int Enemy::getMaxHealthForType(EnemyType t) const {
    switch (t) {
        case EnemyType::Base: return 200;
        case EnemyType::Fast: return 100;
        case EnemyType::Tank: return 800;
        case EnemyType::Shooter: return 250;
        default:              return 200;
    }
}

float Enemy::getHitChance(const Player& player) const {
    if (type != EnemyType::Shooter)
        return 1.0f;

    // Distance based accuracy dropoff for shooters
    float dist = distanceTo(player);

    if (dist < 1.5f) return 0.85f;
    if (dist < 3.0f) return 0.55f;
    if (dist <= 5.0f) return 0.35f;

    return 0.25f;
}

float Enemy::getShieldMultiplier() const {
    switch (type) {
        case EnemyType::Tank: return 1.5f; // More effective vs player shield
        case EnemyType::Shooter: return 0.7f; // Less
        case EnemyType::Fast: return 1.0f;
        case EnemyType::Base: return 1.0f;
        default: return 1.0f;
    }
}

bool Enemy::canAttack(const Player& player) const {
    return distanceTo(player) <= attackRange;
}

void Enemy::handleAttack(float dt, Player& player, AudioManager& audio) {
    animState = EnemyAnimState::Attack;

    // Apply damage on the hit frame
    if (!hasDealtDamageThisAttack && animFrame == attackHitFrame) {

        bool hit = true;

        switch (type) {
            case EnemyType::Tank: audio.playSFX("tank_attack");
            case EnemyType::Shooter: audio.playSFX("shooter_attack");
            case EnemyType::Fast: audio.playSFX("fast_attack");
            case EnemyType::Base: audio.playSFX("base_attack");
            default: audio.playSFX("base_attack");
        }

        // Determine if shooter zombie hits or misses
        if (type == EnemyType::Shooter) {
            float roll = float(rand()) / float(RAND_MAX); // 0.0–1.0
            hit = (roll <= getHitChance(player));
        }

        if (hit) {
            player.applyDamage(attackDamage, getShieldMultiplier());
        }
        // else miss — do nothing (will add sound later)

        hasDealtDamageThisAttack = true;
    }

    // Attack animation finished, go back to chasing
    Animation* currentAnim = isDamaged()
        ? &managerPtr->enemyVisualsDamaged.at(type).animations.at(animState)
        : &managerPtr->enemyVisuals.at(type).animations.at(animState);

    if (animFrame == currentAnim->frames.size() - 1) {
        attackTimer = attackCooldown;

        hasDealtDamageThisAttack = false;
        animFrame = 0;
        animTimer = 0.0f;

        state = EnemyState::Chasing;
    }
}

void Enemy::update(float dt, const Player& player, const Map& map, AudioManager& audio) {
    if (!active) return;

    if (isDead()) {
        if (animState != EnemyAnimState::Death) {
            animState = EnemyAnimState::Death;
            animFrame = 0;
            animTimer = 0.0f;
        }

        updateAnimation(dt);
        return;
    }

    bool seesPlayer = hasLineOfSight(player, map);

    attackTimer -= dt;
    if (attackTimer < 0.0f)
        attackTimer = 0.0f;

    switch (state) {
        case EnemyState::Idle:
            animState = EnemyAnimState::Idle;
            if (seesPlayer) state = EnemyState::Chasing;
            else wander(dt);
            break;

        case EnemyState::Chasing:
            animState = EnemyAnimState::Walk;

            if (seesPlayer) {
                float dist = distanceTo(player);

                if (dist <= attackRange && attackTimer == 0.0f) {
                    state = EnemyState::Attacking;
                    animState = EnemyAnimState::Attack;
                    animFrame = 0;
                    animTimer = 0.0f;
                    hasDealtDamageThisAttack = false;
                    break;
                }

                chasePlayer(dt, player);
                loseSightTimer = 1.0f;
            } else {
                state = EnemyState::Searching;
            }
            break;

        case EnemyState::Searching:
            animState = EnemyAnimState::Walk;
            loseSightTimer -= dt;
            chasePlayer(dt, player);
            if (loseSightTimer <= 0.0f) state = EnemyState::Idle;
            break;

        case EnemyState::Attacking:
            handleAttack(dt, const_cast<Player&>(player), audio);
            break;
    }

    updateAnimation(dt);
}

void Enemy::reset() {
    health = maxHealth;

    active = false;

    state = EnemyState::Idle;
    animState = EnemyAnimState::Idle;

    animFrame = 0;
    animTimer = 0.0f;

    deathAnimFinished = false;

    attackTimer = 0.0f;
    loseSightTimer = 0.0f;
    hasDealtDamageThisAttack = false;

    spritePixels.clear();
    spriteW = 0;
    spriteH = 0;
}

