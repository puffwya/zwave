#include "EnemyManager.h"
#include <cmath>
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
            if (map.data[x][y] == 2) {
                spawnPoints.push_back({ x, y });
            }
        }
    }
}

void EnemyManager::initialize(SDL_Renderer* renderer) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].sprite = IMG_LoadTexture(renderer, "../enemy.png");

        if (!enemies[i].sprite) {
            std::cerr << "Failed to load enemy texture: " << IMG_GetError() << std::endl;
        }
    }
}

// Spawn the *first available* enemy from the pool
Enemy* EnemyManager::spawnEnemy(EnemyType type) {
    if (spawnPoints.empty())
        return nullptr; // no valid spawn tiles

    // Choose a random spawn point
    const auto& pt = spawnPoints[std::rand() % spawnPoints.size()];

    // Find an inactive enemy slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].activate(pt.x, pt.y, type);
            return &enemies[i];
        }
    }

    return nullptr; // pool full
}

void EnemyManager::update(float dt, const Player& player, const Map& map) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy& e = enemies[i];
        if (!e.active) continue;

        e.update(dt, player);

        // OPTIONAL: basic collision with walls
        int px = int(e.x);
        int py = int(e.y);

        if (map.data[px][py] == 1) {
            // push enemy back out of walls
            e.x -= std::cos(e.angle) * e.speed * dt;
            e.y -= std::sin(e.angle) * e.speed * dt;
        }
    }
}

void EnemyManager::deactivateAll() {
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = false;
}

