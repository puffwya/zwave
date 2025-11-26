#ifndef SPRITE_RENDERER_H
#define SPRITE_RENDERER_H

#include <SDL2/SDL.h>
#include "EnemyManager.h"
#include "Player.h"

class SpriteRenderer {
public:
    // tex: loaded SDL_Texture of enemy sprite (assumes full image width = texW, height = texH)
    // zBuffer: array of floats size = screenW (distance to wall per column)
    static void renderEnemies(SDL_Renderer* renderer,
                              EnemyManager& manager, const Player& player,
                              float* zBuffer, int screenWidth, int screenHeight);
};

#endif

