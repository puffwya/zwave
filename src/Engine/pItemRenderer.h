#pragma once
#include <SDL2/SDL.h>
#include "Player.h"

class pItemRenderer {
public:
    static void renderPItem(SDL_Renderer* renderer,
                             SDL_Texture* itemTex,
                             int screenWidth,
                             int screenHeight,
                             ItemType itemType);
};

