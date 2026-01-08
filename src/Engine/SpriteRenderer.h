#pragma once

#include <SDL2/SDL.h>
#include "EnemyManager.h"
#include "Player.h"
#include <vector>

struct CachedTexture {
    int w = 0, h = 0;
    std::vector<uint32_t> pixels;
};

class SpriteRenderer {
public:
    void renderEnemies(
        uint32_t* pixels,
        int screenW,
        int screenH,
        EnemyManager& manager,
        const Player& player,
        float* zBuffer,
        Map& map
    );

private:
    CachedTexture& cacheTexture(SDL_Texture* tex);
};
