#pragma once

#include "EnemyManager.h"
#include "Player.h"
#include "Map.h"

class SpriteRenderer {
public:
    void renderEnemies(
        uint32_t* pixels,
        int screenW,
        int screenH,
        EnemyManager& manager,
        const Player& player,
        float* zBuffer,
        Map& map,
        float colWallTop[]
    );
};

