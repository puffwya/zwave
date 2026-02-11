#pragma once
#include <SDL2/SDL.h>
#include "Player.h"
#include "WeaponTypes.h"

struct AmmoTickStyle {
    int tickW;
    int tickH;
    int spacing;
};

class HUD {
public:
    bool init(SDL_Renderer* renderer);

    void render(SDL_Renderer* renderer,
                const Player& player,
                int screenW,
                int screenH, Weapon& weapon, int currentWave,
                int totalWaves,
                int enemiesRemaining,
                float postWaveTimer);

private:
    void drawDiamondBar(SDL_Renderer* renderer,
                         int cx, int cy, int size,  // diamond center + half-width
                         int value, int maxValue,
                         SDL_Color barColor,
                         SDL_Color frameColor,
                         int thickness = 2);

    void drawReserveAmmo(SDL_Renderer* renderer,
                          int reserve,
                          int x,
                          int y,
                          int scale);

    void drawAmmoTicks(SDL_Renderer* renderer,
                        int clip,
                        int clipSize,
                        int x,
                        int y,
                        int scale,
                        const AmmoTickStyle& style);

    void drawWaveBanner(SDL_Renderer* renderer,
                        int screenW,
                        int screenH,
                        int currentWave,
                        int totalWaves,
                        int enemiesRemaining,
                        float postWaveTimer);

    SDL_Texture* waveTextTexture = nullptr;

    SDL_Texture* enemiesLeftTextTexture = nullptr;

    SDL_Texture* waveStartingTextTexture = nullptr;

    SDL_Color getHealthColor(int value, int maxValue);

    SDL_Color getArmorColor(int value, int maxValue);

    void drawDiamond(SDL_Renderer* renderer, int cx, int cy, int size, SDL_Color color, int thickness = 2);

    void drawHealthCross(SDL_Renderer* renderer, int cx, int cy, int width, int height, SDL_Color color, int thickness);

    void drawArmorShield(SDL_Renderer* renderer, int cx, int cy, int size, SDL_Color color, int thickness = 2);

    // Ammo tick size
    AmmoTickStyle pistolTicks  { 3, 20, 2 };
    AmmoTickStyle shotgunTicks { 8, 22, 5 };
    AmmoTickStyle mgTicks   { 2, 18, 2 };

    SDL_Texture* digitTextures[11]{};
    int digitW = 0;
    int digitH = 0;
};

