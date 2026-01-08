#include "EnemyManager.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <SDL2/SDL_image.h>

EnemyManager::EnemyManager() {
    // ensure all enemies start inactive
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

void EnemyManager::initialize(SDL_Renderer* renderer) {

    enemyTextures[EnemyType::Base] =
        IMG_LoadTexture(renderer, "Assets/enemy_base.png");

    enemyTextures[EnemyType::Tank] =
        IMG_LoadTexture(renderer, "Assets/enemy_tank.png");

    // Optional: fast enemy later
    // enemyTextures[EnemyType::Fast] =
    //     IMG_LoadTexture(renderer, "Assets/enemy_fast.png");

    for (auto& [type, tex] : enemyTextures) {
        if (!tex) {
            std::cerr << "Failed to load enemy texture: "
                      << IMG_GetError() << std::endl;
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        EnemyType type = enemies[i].type;
        if (enemyTextures.count(type))
            enemies[i].sprite = enemyTextures[type];
        else
            enemies[i].sprite = enemyTextures[EnemyType::Base];
    }
}

// Spawn the *first available* enemy from the pool, round-robin spawn
Enemy* EnemyManager::spawnEnemy(EnemyType type) {
    if (spawnPoints.empty())
        return nullptr; // no valid spawn tiles

    // Pick the next spawn point in round-robin fashion
    const auto& pt = spawnPoints[nextSpawnIndex];
    nextSpawnIndex = (nextSpawnIndex + 1) % spawnPoints.size();

    // Find an inactive enemy slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].activate(pt.x, pt.y, type);
            enemies[i].z = 0.0f;
            enemies[i].height = 0.75f;
            return &enemies[i];
        }
    }
            
    return nullptr; // pool full
}

void EnemyManager::update(float dt, const Player& player, const Map& map) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy& e = enemies[i];
        if (!e.active) continue;

        // update enemy movement state
        e.update(dt, player, map);

        // wall collision
        int px = int(e.x);
        int py = int(e.y);
        if (map.get(px,py).type == Map::TileType::Wall) {
            e.x -= std::cos(e.angle) * e.speed * dt;
            e.y -= std::sin(e.angle) * e.speed * dt;
        }

        // enemy-enemy collision
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (i == j) continue;
            Enemy& other = enemies[j];
            if (!other.active) continue;

            float dx = e.x - other.x;
            float dy = e.y - other.y;
            float distSq = dx*dx + dy*dy;
            float minDist = 0.8f; // minimal distance between enemies
            if (distSq < minDist*minDist) {
                float dist = std::sqrt(distSq);
                if (dist > 0.0f) {
                    float push = (minDist - dist) / 2.0f; // split push
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

void EnemyManager::deactivateAll() {
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = false;
}

