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

    // convert angle into direction vector
    float dirX = std::cos(player.angle);
    float dirY = std::sin(player.angle);

    // camera plane (controls FOV)
    const float fovScale = 0.66f;  // 66° FOV
    float planeX = -dirY * fovScale;
    float planeY =  dirX * fovScale;


    // Render enemies in sorted order
    for (int i = 0; i < count; i++) {
        Enemy* e = activeEnemies[i];

        float dx = e->x - player.x;
        float dy = e->y - player.y;

        // Transform into camera space
        float invDet = 1.0f / (planeX * dirY - dirX * planeY);

        float transformX = invDet * ( dirY  * dx - dirX * dy );
        float transformY = invDet * (-planeY * dx + planeX * dy );

        // If behind the player, skip
        if (transformY <= 0) continue;

        int screenX = int((screenWidth / 2) * (1 + transformX / transformY));
        float dist = std::sqrt(dx*dx + dy*dy);

        int spriteSize = std::max(10, int(screenHeight / dist));

        int texW, texH;
        SDL_QueryTexture(e->sprite, NULL, NULL, &texW, &texH);

        SDL_Rect dst{ screenX - spriteSize / 2, screenHeight / 2 - spriteSize / 2, spriteSize, spriteSize };
        SDL_Rect src{ 0, 0, texW, texH };

        SDL_RenderCopy(renderer, e->sprite, &src, &dst);
    }
}

