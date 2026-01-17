#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include <vector>
#include "Enemy.h"
#include "Map.h"
#include <unordered_map>
#include "Texture.h"
#include <SDL2/SDL.h>

struct EnemySprite {
    int w = 0;
    int h = 0;
    std::vector<uint32_t> pixels;
};

class EnemyManager {
public:
    static const int MAX_ENEMIES = 128;

    // Store spawn locations from the map (tile value = 2)
    struct SpawnPoint { int x, y; };

    EnemyManager();

    Enemy enemies[MAX_ENEMIES];

    void scanMapForSpawnPoints(const Map& map);

    Enemy* spawnEnemy(EnemyType type);

    // Load all enemy assets at once
    void loadEnemyAssets();
    const EnemySprite& getSprite(EnemyType type) const;

    void update(float dt, const Player& player, const Map& map);

    bool hasActiveEnemies() const;

    // Load textures once per enemy type
    std::unordered_map<EnemyType, SDL_Texture*> enemyTextures;

    // Safety check to clear all active enemies at wave end
    void deactivateAll();

    int getActiveEnemyCount() const;

private:
    std::vector<SpawnPoint> spawnPoints;
    int nextSpawnIndex = 0;  // keeps track of next spawn point
    std::unordered_map<EnemyType, EnemySprite> spriteCache;
};

#endif

