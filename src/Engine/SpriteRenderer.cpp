// Draw Sprites separate of raycasting system rotated to always face the player
// Sprites are drawn in screen-space and its size depends on distance from player
// Its screen x-position depends on the angle to the player

#include "SpriteRenderer.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <SDL2/SDL.h>

struct SpriteDraw {
    Enemy* e;
    float dist;
};

void SpriteRenderer::renderEnemies(
    SDL_Renderer* renderer,
    EnemyManager& manager,
    const Player& player,
    float* zBuffer,
    int screenWidth,
    int screenHeight
) {
    // Array of pointers to active enemies
    Enemy* activeEnemies[EnemyManager::MAX_ENEMIES];
    float distances[EnemyManager::MAX_ENEMIES];
    int count = 0;

    // Collect active enemies and their distances
    for (int i = 0; i < manager.MAX_ENEMIES; i++) {
        Enemy& e = manager.enemies[i];
        if (!e.active || !e.sprite) continue;

        float dx = e.x - player.x;
        float dy = e.y - player.y;
        distances[count] = std::sqrt(dx*dx + dy*dy);
        activeEnemies[count] = &e;
        count++;
    }

    // Sort pointers by distance descending (farthest first)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (distances[i] < distances[j]) {
                std::swap(distances[i], distances[j]);
                std::swap(activeEnemies[i], activeEnemies[j]);
            }
        }
    }

    // Render enemies in sorted order
    for (int i = 0; i < count; i++) {
        Enemy* e = activeEnemies[i];

        float dx = e->x - player.x;
        float dy = e->y - player.y;
        float dist = std::sqrt(dx*dx + dy*dy);

        float spriteAngle = std::atan2(dy, dx) - player.angle;
        if (spriteAngle < -M_PI) spriteAngle += 2*M_PI;
        if (spriteAngle >  M_PI) spriteAngle -= 2*M_PI;

        if (std::fabs(spriteAngle) > M_PI / 2) continue;

        int texW, texH;
        SDL_QueryTexture(e->sprite, NULL, NULL, &texW, &texH);

        int screenX = int(std::tan(spriteAngle) * (screenWidth / 2) + screenWidth / 2);
        int spriteSize = std::max(10, int(screenHeight / dist));

        SDL_Rect dst{ screenX - spriteSize / 2, screenHeight / 2 - spriteSize / 2, spriteSize, spriteSize };
        SDL_Rect src{ 0, 0, texW, texH };

        SDL_RenderCopy(renderer, e->sprite, &src, &dst);
    }
}

