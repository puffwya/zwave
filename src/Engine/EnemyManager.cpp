#include "EnemyManager.h"
#include "Player.h"
#include <random>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <SDL2/SDL_image.h>

EnemyManager::EnemyManager() {
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = false;
}

void EnemyManager::scanMapForSpawnPoints(const Map& map) {
    spawnPoints.clear();
    for (int x = 0; x < map.SIZE; x++) {
        for (int y = 0; y < map.SIZE; y++) {
            if (map.get(x,y).type == Map::TileType::Spawn) {
                spawnPoints.push_back({ x, y });
            }
        }
    }
}

bool loadSpriteFrame(const std::string& path, SpriteFrame& out) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) return false;

    SDL_Surface* formatted =
        SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);
    if (!formatted) return false;

    out.w = formatted->w;
    out.h = formatted->h;
    out.pixels.resize(out.w * out.h);
    std::memcpy(out.pixels.data(), formatted->pixels, out.w * out.h * 4);

    SDL_FreeSurface(formatted);
    return true;
}

Animation loadAnimation(const std::vector<std::string>& paths, float frameTime) {
    Animation anim;
    anim.frameDuration = frameTime;

    for (const auto& p : paths) {
        SpriteFrame frame;
        if (!loadSpriteFrame(p, frame)) {
            std::cerr << "FAILED TO LOAD FRAME: " << p << std::endl;
        } else {
            anim.frames.push_back(std::move(frame));
        }
    }
    return anim;
}

void EnemyManager::loadEnemyAssets() {
    // Base Enemy
    EnemyVisual base;

    // Base
    base.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Base/walk_0.png",
            "Assets/Enemies/Base/walk_1.png",
            "Assets/Enemies/Base/walk_2.png",
            "Assets/Enemies/Base/walk_3.png",
            "Assets/Enemies/Base/walk_4.png"
        }, 0.12f);

    base.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Base/chase_0.png",
            "Assets/Enemies/Base/chase_1.png",
            "Assets/Enemies/Base/chase_2.png",
            "Assets/Enemies/Base/chase_3.png",
            "Assets/Enemies/Base/chase_4.png"
        }, 0.12f);

     base.animations[EnemyAnimState::Attack] =
        loadAnimation({ 
            "Assets/Enemies/Base/attack_0.png",
            "Assets/Enemies/Base/attack_1.png",
            "Assets/Enemies/Base/attack_2.png",
            "Assets/Enemies/Base/attack_3.png",  
            "Assets/Enemies/Base/attack_4.png"
        }, 0.12f);

    enemyVisuals[EnemyType::Base] = std::move(base);

    EnemyVisual baseDamaged;

    baseDamaged.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Base/walk_damaged_0.png",
            "Assets/Enemies/Base/walk_damaged_1.png",
            "Assets/Enemies/Base/walk_damaged_2.png",
            "Assets/Enemies/Base/walk_damaged_3.png",
            "Assets/Enemies/Base/walk_damaged_4.png"
        }, 0.12f);

    baseDamaged.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Base/chase_damaged_0.png",
            "Assets/Enemies/Base/chase_damaged_1.png",
            "Assets/Enemies/Base/chase_damaged_2.png",
            "Assets/Enemies/Base/chase_damaged_3.png",
            "Assets/Enemies/Base/chase_damaged_4.png"
        }, 0.12f);

    baseDamaged.animations[EnemyAnimState::Attack] =
        loadAnimation({
            "Assets/Enemies/Base/attack_damaged_0.png",
            "Assets/Enemies/Base/attack_damaged_1.png",
            "Assets/Enemies/Base/attack_damaged_2.png",
            "Assets/Enemies/Base/attack_damaged_3.png",
            "Assets/Enemies/Base/attack_damaged_4.png"
        }, 0.12f);

    baseDamaged.animations[EnemyAnimState::Death] =
        loadAnimation({
            "Assets/Enemies/Base/death_damaged_0.png",
            "Assets/Enemies/Base/death_damaged_1.png",
            "Assets/Enemies/Base/death_damaged_2.png",
            "Assets/Enemies/Base/death_damaged_3.png",
            "Assets/Enemies/Base/death_damaged_4.png"
        }, 0.12f);

    enemyVisualsDamaged[EnemyType::Base] = std::move(baseDamaged);

    // Shooter
    EnemyVisual shooter;

    shooter.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Shooter/walk_0.png",
            "Assets/Enemies/Shooter/walk_1.png",
            "Assets/Enemies/Shooter/walk_2.png",
            "Assets/Enemies/Shooter/walk_3.png",
            "Assets/Enemies/Shooter/walk_3.png"
        }, 0.12f);

    shooter.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Shooter/chase_0.png",
            "Assets/Enemies/Shooter/chase_1.png",
            "Assets/Enemies/Shooter/chase_2.png",
            "Assets/Enemies/Shooter/chase_3.png",
            "Assets/Enemies/Shooter/chase_4.png"
        }, 0.12f);

    shooter.animations[EnemyAnimState::Attack] =
        loadAnimation({ 
            "Assets/Enemies/Shooter/attack_0.png",
            "Assets/Enemies/Shooter/attack_1.png",
            "Assets/Enemies/Shooter/attack_2.png",
            "Assets/Enemies/Shooter/attack_3.png",  
            "Assets/Enemies/Shooter/attack_4.png"
        }, 0.12f);

    enemyVisuals[EnemyType::Shooter] = std::move(shooter);

    EnemyVisual shooterDamaged;

    shooterDamaged.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Shooter/walk_damaged_0.png",
            "Assets/Enemies/Shooter/walk_damaged_1.png",
            "Assets/Enemies/Shooter/walk_damaged_2.png",
            "Assets/Enemies/Shooter/walk_damaged_3.png",
            "Assets/Enemies/Shooter/walk_damaged_4.png"
        }, 0.12f);

    shooterDamaged.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Shooter/chase_damaged_0.png",
            "Assets/Enemies/Shooter/chase_damaged_1.png",
            "Assets/Enemies/Shooter/chase_damaged_2.png",
            "Assets/Enemies/Shooter/chase_damaged_3.png",
            "Assets/Enemies/Shooter/chase_damaged_4.png"
        }, 0.12f);

    shooterDamaged.animations[EnemyAnimState::Attack] = 
        loadAnimation({
            "Assets/Enemies/Shooter/attack_damaged_0.png",
            "Assets/Enemies/Shooter/attack_damaged_1.png",
            "Assets/Enemies/Shooter/attack_damaged_2.png",
            "Assets/Enemies/Shooter/attack_damaged_3.png",
            "Assets/Enemies/Shooter/attack_damaged_4.png"
        }, 0.12f);

    shooterDamaged.animations[EnemyAnimState::Death] =
        loadAnimation({
            "Assets/Enemies/Shooter/death_damaged_0.png",
            "Assets/Enemies/Shooter/death_damaged_1.png",
            "Assets/Enemies/Shooter/death_damaged_2.png",
            "Assets/Enemies/Shooter/death_damaged_3.png",
            "Assets/Enemies/Shooter/death_damaged_4.png"
        }, 0.12f);

    enemyVisualsDamaged[EnemyType::Shooter] = std::move(shooterDamaged);

    // Tank
    EnemyVisual tank;

    tank.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Tank/walk_0.png",
            "Assets/Enemies/Tank/walk_1.png",
            "Assets/Enemies/Tank/walk_2.png",
            "Assets/Enemies/Tank/walk_3.png",
            "Assets/Enemies/Tank/walk_4.png"
        }, 0.12f);

    tank.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Tank/chase_0.png",
            "Assets/Enemies/Tank/chase_1.png",
            "Assets/Enemies/Tank/chase_2.png",
            "Assets/Enemies/Tank/chase_3.png",
            "Assets/Enemies/Tank/chase_4.png"
        }, 0.12f);

    tank.animations[EnemyAnimState::Attack] =
        loadAnimation({
            "Assets/Enemies/Tank/attack_0.png",
            "Assets/Enemies/Tank/attack_1.png",
            "Assets/Enemies/Tank/attack_2.png",
            "Assets/Enemies/Tank/attack_3.png",
            "Assets/Enemies/Tank/attack_4.png"
        }, 0.12f);

    enemyVisuals[EnemyType::Tank] = std::move(tank);

    EnemyVisual tankDamaged;

    tankDamaged.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Tank/walk_damaged_0.png",
            "Assets/Enemies/Tank/walk_damaged_1.png",
            "Assets/Enemies/Tank/walk_damaged_2.png",
            "Assets/Enemies/Tank/walk_damaged_3.png",
            "Assets/Enemies/Tank/walk_damaged_4.png"
        }, 0.12f);

    tankDamaged.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Tank/chase_damaged_0.png",
            "Assets/Enemies/Tank/chase_damaged_1.png",
            "Assets/Enemies/Tank/chase_damaged_2.png",
            "Assets/Enemies/Tank/chase_damaged_3.png",
            "Assets/Enemies/Tank/chase_damaged_4.png"
        }, 0.12f);

    tankDamaged.animations[EnemyAnimState::Attack] =
        loadAnimation({
            "Assets/Enemies/Tank/attack_damaged_0.png",  
            "Assets/Enemies/Tank/attack_damaged_1.png", 
            "Assets/Enemies/Tank/attack_damaged_2.png",
            "Assets/Enemies/Tank/attack_damaged_3.png",
            "Assets/Enemies/Tank/attack_damaged_4.png" 
        }, 0.12f);

    tankDamaged.animations[EnemyAnimState::Death] =
        loadAnimation({
            "Assets/Enemies/Tank/death_damaged_0.png",
            "Assets/Enemies/Tank/death_damaged_1.png",
            "Assets/Enemies/Tank/death_damaged_2.png",
            "Assets/Enemies/Tank/death_damaged_3.png",
            "Assets/Enemies/Tank/death_damaged_4.png"
        }, 0.12f);

    enemyVisualsDamaged[EnemyType::Tank] = std::move(tankDamaged);

    // Fast
    EnemyVisual fast;

    fast.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Fast/walk_0.png",
            "Assets/Enemies/Fast/walk_1.png",
            "Assets/Enemies/Fast/walk_2.png",
            "Assets/Enemies/Fast/walk_3.png",
            "Assets/Enemies/Fast/walk_4.png"
        }, 0.12f);

    fast.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Fast/chase_0.png",
            "Assets/Enemies/Fast/chase_1.png",
            "Assets/Enemies/Fast/chase_2.png",
            "Assets/Enemies/Fast/chase_3.png",
            "Assets/Enemies/Fast/chase_4.png"
        }, 0.12f);

    fast.animations[EnemyAnimState::Attack] =
        loadAnimation({
            "Assets/Enemies/Fast/attack_0.png",   
            "Assets/Enemies/Fast/attack_1.png", 
            "Assets/Enemies/Fast/attack_2.png",
            "Assets/Enemies/Fast/attack_3.png",   
            "Assets/Enemies/Fast/attack_4.png"
        }, 0.12f);

    enemyVisuals[EnemyType::Fast] = std::move(fast);

    EnemyVisual fastDamaged;

    fastDamaged.animations[EnemyAnimState::Idle] =
        loadAnimation({
            "Assets/Enemies/Fast/walk_damaged_0.png",
            "Assets/Enemies/Fast/walk_damaged_1.png",
            "Assets/Enemies/Fast/walk_damaged_2.png",
            "Assets/Enemies/Fast/walk_damaged_3.png",
            "Assets/Enemies/Fast/walk_damaged_4.png"
        }, 0.12f);

    fastDamaged.animations[EnemyAnimState::Walk] =
        loadAnimation({
            "Assets/Enemies/Fast/chase_damaged_0.png",
            "Assets/Enemies/Fast/chase_damaged_1.png",
            "Assets/Enemies/Fast/chase_damaged_2.png",
            "Assets/Enemies/Fast/chase_damaged_3.png",
            "Assets/Enemies/Fast/chase_damaged_4.png"
        }, 0.12f);

    fastDamaged.animations[EnemyAnimState::Attack] =  
        loadAnimation({
            "Assets/Enemies/Fast/attack_damaged_0.png",
            "Assets/Enemies/Fast/attack_damaged_1.png",
            "Assets/Enemies/Fast/attack_damaged_2.png",
            "Assets/Enemies/Fast/attack_damaged_3.png",
            "Assets/Enemies/Fast/attack_damaged_4.png"
        }, 0.12f);

    fastDamaged.animations[EnemyAnimState::Death] =
        loadAnimation({
            "Assets/Enemies/Fast/death_damaged_0.png",
            "Assets/Enemies/Fast/death_damaged_1.png",
            "Assets/Enemies/Fast/death_damaged_2.png",
            "Assets/Enemies/Fast/death_damaged_3.png",
            "Assets/Enemies/Fast/death_damaged_4.png"
        }, 0.12f);

    enemyVisualsDamaged[EnemyType::Fast] = std::move(fastDamaged);
}

Enemy* EnemyManager::spawnEnemy(EnemyType type) {
    if (spawnPoints.empty()) return nullptr;

    const auto& pt = spawnPoints[nextSpawnIndex];
    nextSpawnIndex = (nextSpawnIndex + 1) % spawnPoints.size();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            Enemy& e = enemies[i];
            e.reset();
            e.activate(pt.x, pt.y, type, *this);
            e.z = 0.0f;
            e.height = 0.75f;
            return &e;
        }
    }

    return nullptr;
}

void EnemyManager::trySpawnAmmoDrop(
    const Enemy& e,
    const Player& player,
    PickupManager& pickupManager)
{
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dropRoll(0.0f, 1.0f);

    float dropChance = 0.65f;
    if (dropRoll(rng) > dropChance)
        return;

    WeaponType weaponType;

    switch (player.currentItem)
    {
        case ItemType::Pistol:
            weaponType = WeaponType::Pistol;
            break;

        case ItemType::Shotgun:
            weaponType = WeaponType::Shotgun;
            break;

        case ItemType::Mg:
            weaponType = WeaponType::Mg;
            break;

        default:
            return; // no valid weapon equipped
    }

    pickupManager.addPickup(
        e.x,
        e.y,
        0.0f,
        PickupType::Ammo,
        weaponType
    );
}

void EnemyManager::update(float dt, const Player& player, PickupManager& pickupManager, const Map& map, AudioManager& audio) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy& e = enemies[i];
        if (!e.active) continue;

        // Save previous position
        float oldX = e.x;
        float oldY = e.y;

        e.update(dt, player, map, audio, e.type);

        // Death logic
        if (e.deathJustFinished) {
            trySpawnAmmoDrop(e, player, pickupManager);
            // Keep track of enemies killed
            enemiesKilled += 1;
            e.deathJustFinished = false;
        }

        // Wall Collision with height check
        int px = int(e.x);
        int py = int(e.y);

        const auto& tile = map.get(px, py);

        if (tile.type == Map::TileType::Wall) {
            float wallHeight = tile.height;
            float stepHeight = 0.25f; // enemy step capability

            if (wallHeight > stepHeight) {
                // Solid wall, block movement
                e.x = oldX;
                e.y = oldY;

                // Re-roll wander direction if idle
                if (e.state == EnemyState::Idle) {
                    e.wanderTimer = 0.0f;
                }
            }
            else {
                // Walkable wall, step onto it
                e.z = wallHeight;
            }
        }
        else {
            // Not a wall, return to ground
            e.z = 0.0f;
        }

        // Enemy separation
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (i == j) continue;
            Enemy& other = enemies[j];
            if (!other.active) continue;

            float dx = e.x - other.x;
            float dy = e.y - other.y;
            float distSq = dx*dx + dy*dy;
            float minDist = 0.8f;

            if (distSq < minDist * minDist) {
                float dist = std::sqrt(distSq);
                if (dist > 0.0f) {
                    float push = (minDist - dist) * 0.5f;
                    dx /= dist;
                    dy /= dist;

                    e.x += dx * push;
                    e.y += dy * push;
                    other.x -= dx * push;
                    other.y -= dy * push;
                }
            }
        }
        // Failsafe
        e.x = std::clamp(e.x, 1.0f, map.SIZE - 2.0f);
        e.y = std::clamp(e.y, 1.0f, map.SIZE - 2.0f);
    }
}

bool EnemyManager::hasActiveEnemies() const
{
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].active &&
            !enemies[i].deathAnimFinished)
            return true;

    return false;
}

int EnemyManager::getActiveEnemyCount() const {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].active && enemies[i].deathAnimFinished == false) count++;
    return count;
}

void EnemyManager::deactivateAll() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
        enemies[i].reset();
    }
}

