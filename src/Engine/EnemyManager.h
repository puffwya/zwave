#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include <vector>
#include "Enemy.h"
#include "Map.h"
#include <unordered_map>
#include <SDL2/SDL.h>

struct SpriteFrame {
    int w = 0;
    int h = 0;
    std::vector<uint32_t> pixels;
};

struct Animation {
    std::vector<SpriteFrame> frames;
    float frameDuration = 0.1f; // seconds per frame
};

struct EnemyVisual {
    std::unordered_map<EnemyAnimState, Animation> animations;
};

class EnemyManager {
public:
    static const int MAX_ENEMIES = 128;

    struct SpawnPoint { int x, y; };

    EnemyManager();

    Enemy enemies[MAX_ENEMIES];

    void scanMapForSpawnPoints(const Map& map);
    Enemy* spawnEnemy(EnemyType type);

    void loadEnemyAssets();
    void update(float dt, const Player& player, const Map& map, AudioManager& audio);
    void updateEnemy(Enemy& e, float dt);

    bool hasActiveEnemies() const;
    int getActiveEnemyCount() const;
    void deactivateAll();

    std::unordered_map<EnemyType, EnemyVisual> enemyVisuals;

    std::unordered_map<EnemyType, EnemyVisual> enemyVisualsDamaged;

private:
    std::vector<SpawnPoint> spawnPoints;
    int nextSpawnIndex = 0;
};

#endif

