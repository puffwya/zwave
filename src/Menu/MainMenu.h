#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../Engine/GameState.h"

struct AshParticle {
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
};

class MainMenu {
public:
    bool init(SDL_Renderer* renderer, int screenW, int screenH);
    void handleInput(const SDL_Event& e, GameState& gs, bool& running, bool& mRunning);
    void updateCursor();
    void activateSelected(GameState& gs, bool& running, bool& mRunning);
    void render(SDL_Renderer* renderer);
    void shutdown();

    enum MenuItem {
        MENU_START = 0,
        MENU_OPTIONS,
        MENU_QUIT,
        MENU_COUNT
    };

    int selectedIndex = MENU_START;

    SDL_Rect menuRects[MENU_COUNT];

    SDL_Rect cursorRect{};

    std::vector<AshParticle> ashParticles;

private:
    SDL_Texture* mainBgTexture = nullptr;
    SDL_Rect mainBgRect{};

    SDL_Texture* mainLogoFgTexture = nullptr;
    SDL_Texture* mainLogoBgTexture = nullptr; // Glowing Background 
    SDL_Rect mainLogoRect{};

    SDL_Texture* startTexture = nullptr;
    SDL_Rect startRect{};

    SDL_Texture* optionsTexture = nullptr;
    SDL_Rect optionsRect{};

    SDL_Texture* quitTexture = nullptr;
    SDL_Rect quitRect{};

    SDL_Texture* cursorTexture = nullptr;

    int screenW = 0;
    int screenH = 0;

    void spawnAshParticle();
    void updateAndRenderAsh(SDL_Renderer* renderer, float dt);
};

