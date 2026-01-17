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

// Helper for loadEnemyAssets
bool loadEnemySprite(Enemy& e, const std::string& path) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cerr << "Failed to load sprite: " << path << " | " 
                  << IMG_GetError() << std::endl;
        return false;
    }

    // Convert surface to ARGB8888 format (if needed)
    SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);
    if (!formatted) {
        std::cerr << "Failed to convert sprite to ARGB8888: " << path << std::endl;
        return false;
    }

    e.spriteW = formatted->w;
    e.spriteH = formatted->h;
    e.spritePixels.resize(e.spriteW * e.spriteH);

    // Copy pixels into vector
    std::memcpy(e.spritePixels.data(), formatted->pixels, e.spriteW * e.spriteH * 4);

    SDL_FreeSurface(formatted);
    return true;
}

// Loads each enemy types assets
void EnemyManager::loadEnemyAssets() {
    auto load = [&](EnemyType type, const std::string& path) {
        EnemySprite sprite;
        Enemy temp; // dummy enemy for loader

        if (!loadEnemySprite(temp, path)) {
            std::cerr << "Failed to load sprite for type\n";
            return;
        }

        sprite.w = temp.spriteW;
        sprite.h = temp.spriteH;
        sprite.pixels = std::move(temp.spritePixels);

        spriteCache[type] = std::move(sprite);
    };

    load(EnemyType::Base, "Assets/enemy_base.png");
    load(EnemyType::Tank, "Assets/enemy_tank.png");
}

// Spawn the *first available* enemy from the pool, round-robin spawn
Enemy* EnemyManager::spawnEnemy(EnemyType type) {
    if (spawnPoints.empty())
        return nullptr;
    
    // Pick the next spawn point (round-robin)
    const auto& pt = spawnPoints[nextSpawnIndex];
    nextSpawnIndex = (nextSpawnIndex + 1) % spawnPoints.size();

    // Find an inactive enemy slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            Enemy& e = enemies[i];

            // Existing behavior
            e.activate(pt.x, pt.y, type);
            e.z = 0.0f;
            e.height = 0.75f;

            // Assign sprite by type
            auto it = spriteCache.find(type);
            if (it != spriteCache.end()) {
                e.spriteW = it->second.w;
                e.spriteH = it->second.h;
                e.spritePixels = it->second.pixels;
            } else {
                // Safety fallback
                e.spritePixels.clear();
                e.spriteW = e.spriteH = 0;
            }

            return &e;
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

bool EnemyManager::hasActiveEnemies() const {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        const Enemy& e = enemies[i];
        if (e.active)
            return true;
    }
    return false;
}

int EnemyManager::getActiveEnemyCount() const {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) count++;
    }
    return count;
}

void EnemyManager::deactivateAll() {
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = false;
}

