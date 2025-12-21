#pragma once
#include <SDL2/SDL.h>
#include "Player.h"

class HUD {
public:
    bool loadDigitTextures(SDL_Renderer* renderer);

    void render(SDL_Renderer* renderer,
                const Player& player,
                int screenW,
                int screenH);

private:
    void drawBar(SDL_Renderer* renderer,
                 int x, int y, int w, int h,
                 int value, int maxValue,
                 SDL_Color color);

    void drawDigits(SDL_Renderer* renderer,
                    int value,
                    int x,
                    int y,
                    bool rightAlign);

    SDL_Texture* digitTextures[10]{};
    int digitW = 0;
    int digitH = 0;
};

