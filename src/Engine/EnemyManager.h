#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include <vector>
#include "Enemy.h"
#include "Map.h"

class EnemyManager {
public:
    static const int MAX_ENEMIES = 128;

    // Store spawn locations from the map (tile value = 2)
    struct SpawnPoint { int x, y; };

    EnemyManager();

    Enemy enemies[MAX_ENEMIES];

    void scanMapForSpawnPoints(const Map& map);

    Enemy* spawnEnemy(EnemyType type);

    void initialize(SDL_Renderer* renderer);

    void update(float dt, const Player& player, const Map& map);

    // Safety check to clear all active enemies at wave end
    void deactivateAll();

private:
    std::vector<SpawnPoint> spawnPoints;
    int nextSpawnIndex = 0;  // keeps track of next spawn point
};

#endif

