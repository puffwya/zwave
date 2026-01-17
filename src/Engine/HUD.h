#pragma once
#include <SDL2/SDL.h>
#include "Player.h"
#include "WeaponTypes.h"

class HUD {
public:
    bool init(SDL_Renderer* renderer);

    void render(SDL_Renderer* renderer,
                const Player& player,
                int screenW,
                int screenH, Weapon& weapon, int currentWave,
                int totalWaves,
                int enemiesRemaining);

private:
    void drawDiamondBar(SDL_Renderer* renderer,
                         int cx, int cy, int size,  // diamond center + half-width
                         int value, int maxValue,
                         SDL_Color barColor,
                         SDL_Color frameColor,
                         int thickness = 2);

    void drawAmmo(SDL_Renderer* renderer,
                   int clip,
                   int reserve,
                   int x,
                   int y,
                   int scale);

    void drawWaveBanner(SDL_Renderer* renderer,
                        int screenW,
                        int screenH,
                        int currentWave,
                        int totalWaves,
                        int enemiesRemaining);

    SDL_Texture* waveTextTexture = nullptr;

    SDL_Texture* enemiesLeftTextTexture = nullptr;

    SDL_Color getHealthColor(int value, int maxValue);

    SDL_Color getArmorColor(int value, int maxValue);

    void drawDiamond(SDL_Renderer* renderer, int cx, int cy, int size, SDL_Color color, int thickness = 2);

    void drawHealthCross(SDL_Renderer* renderer, int cx, int cy, int width, int height, SDL_Color color, int thickness);

    void drawArmorShield(SDL_Renderer* renderer, int cx, int cy, int size, SDL_Color color, int thickness = 2);

    void drawRoundedTopLeft(SDL_Renderer* renderer, SDL_Rect rect, int radius, SDL_Color color);

    SDL_Texture* digitTextures[11]{};
    int digitW = 0;
    int digitH = 0;
};

