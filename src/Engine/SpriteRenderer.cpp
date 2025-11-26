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

    for (int i = 0; i < manager.MAX_ENEMIES; i++) {
        Enemy& e = manager.enemies[i];
        if (!e.active) continue;
        if (!e.sprite) continue;

        // For sprite size based on distance
        float dx = e.x - player.x;
        float dy = e.y - player.y;
        float dist = std::sqrt(dx*dx + dy*dy);

        // Calculate angle relative to where player is looking giving the angular offset from the player's viewing 
        // direction, ex. if spriteAngle = 0 -> sprite is directly in the center of the screen
        float spriteAngle = std::atan2(dy, dx) - player.angle;

        // Wrap the angle into -PI..PI range so we don't get weird wraparound issues (more circular)
        if (spriteAngle < -M_PI) spriteAngle += 2*M_PI;
        if (spriteAngle >  M_PI) spriteAngle -= 2*M_PI;

        // Only sprites in the front 180 degrees are drawn (will need adjusted later)
        if (std::fabs(spriteAngle) > M_PI / 2) continue;

        // Query this enemy's texture
        int texW, texH;
        SDL_QueryTexture(e.sprite, NULL, NULL, &texW, &texH);

        // Convert spriteAngle into screen position... tan(spriteAngle) -> gives how far left/right the sprite 
        // appears on the projection plane, multiply by half screen width to spread out horizontally, then adding 
        // half screen width centers the projection
        int screenX = int(std::tan(spriteAngle) * (screenWidth / 2) + screenWidth / 2);

        // Calculate sprite size based on distance (farther == smaller, closer == larger)
        int spriteSize = std::max(10, int(screenHeight / dist));

        // Convert to an SDL_Rect to draw
        SDL_Rect dst{
            screenX - spriteSize / 2,
            screenHeight / 2 - spriteSize / 2,
            spriteSize,
            spriteSize
        };

        SDL_Rect src{ 0, 0, texW, texH };

        // Drawn after the world
        SDL_RenderCopy(renderer, e.sprite, &src, &dst);
    }
}

