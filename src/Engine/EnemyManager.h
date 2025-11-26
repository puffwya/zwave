#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include <vector>
#include "Enemy.h"
#include "Map.h"

class EnemyManager {
public:
    static const int MAX_ENEMIES = 128;

    Enemy enemies[MAX_ENEMIES];

    // Store spawn locations from the map (tile value = 2)
    struct SpawnPoint { int x, y; };
    std::vector<SpawnPoint> spawnPoints;

    EnemyManager();

    void scanMapForSpawnPoints(const Map& map);

    Enemy* spawnEnemy(EnemyType type);

    void initialize(SDL_Renderer* renderer);

    void update(float dt, const Player& player, const Map& map);

    // Optional: Clear all active enemies at wave end
    void deactivateAll();
};

#endif

