#include "EnemyManager.h"
#include <cmath>
#include <cstring>
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

    std::cerr << "Loaded animation with " << anim.frames.size() << " frames\n";
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

    enemyVisualsDamaged[EnemyType::Tank] = std::move(tankDamaged);
}

Enemy* EnemyManager::spawnEnemy(EnemyType type) {
    if (spawnPoints.empty()) return nullptr;

    const auto& pt = spawnPoints[nextSpawnIndex];
    nextSpawnIndex = (nextSpawnIndex + 1) % spawnPoints.size();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            Enemy& e = enemies[i];
            e.activate(pt.x, pt.y, type, *this);
            e.z = 0.0f;
            e.height = 0.75f;
            return &e;
        }
    }

    return nullptr;
}

void EnemyManager::update(float dt, const Player& player, const Map& map) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy& e = enemies[i];
        if (!e.active) continue;

        e.update(dt, player, map);

        int px = int(e.x);
        int py = int(e.y);
        if (map.get(px,py).type == Map::TileType::Wall) {
            e.x -= std::cos(e.angle) * e.speed * dt;
            e.y -= std::sin(e.angle) * e.speed * dt;
        }

        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (i == j) continue;
            Enemy& other = enemies[j];
            if (!other.active) continue;

            float dx = e.x - other.x;
            float dy = e.y - other.y;
            float distSq = dx*dx + dy*dy;
            float minDist = 0.8f;
            if (distSq < minDist*minDist) {
                float dist = std::sqrt(distSq);
                if (dist > 0.0f) {
                    float push = (minDist - dist) / 2.0f;
                    dx /= dist;
                    dy /= dist;
                    e.x += dx * push;
                    e.y += dy * push;
                    other.x -= dx * push;
                    other.y -= dy * push;
                }
            }
        }
    }
}

void EnemyManager::updateEnemy(Enemy& e, float dt) {
    e.animState = e.attacking ? EnemyAnimState::Attack : EnemyAnimState::Walk;

    const EnemyVisual& visual = enemyVisuals[e.type];
    const Animation& anim = visual.animations.at(e.animState);

    e.animTimer += dt;
    if (e.animTimer >= anim.frameDuration) {
        e.animTimer -= anim.frameDuration;
        e.animFrame = (e.animFrame + 1) % anim.frames.size();
    }
}

bool EnemyManager::hasActiveEnemies() const {
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].active) return true;
    return false;
}

int EnemyManager::getActiveEnemyCount() const {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++)
        if (enemies[i].active) count++;
    return count;
}

void EnemyManager::deactivateAll() {
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = false;
}

