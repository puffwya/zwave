#pragma once
#include <SDL2/SDL.h>
#include "Player.h"
#include "WeaponTypes.h"

class HUD {
public:
    bool loadDigitTextures(SDL_Renderer* renderer);

    void render(SDL_Renderer* renderer,
                const Player& player,
                int screenW,
                int screenH, Weapon& weapon);

private:
    void drawBar(SDL_Renderer* renderer,
                 int x, int y, int w, int h,
                 int value, int maxValue,
                 SDL_Color color);

    void drawAmmo(SDL_Renderer* renderer,
                   int clip,
                   int reserve,
                   int x,
                   int y,
                   int scale);

    SDL_Texture* digitTextures[11]{};
    int digitW = 0;
    int digitH = 0;
};

