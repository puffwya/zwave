#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../Engine/GameState.h"
#include "../settings/GameSettings.h"

struct AshParticle {
    float x, y;
    float vx, vy;
    float life;
    float maxLife;
    float size;
};

class MainMenu {
public:
    bool init(SDL_Renderer* renderer, int screenW, int screenH);
    void handleInput(const SDL_Event& e, GameState& gs, bool& running, bool& mRunning, Difficulty& difficulty);
    void updateCursor(float dt);
    void activateSelected(GameState& gs, bool& running, bool& mRunning, Difficulty& difficulty);
    void render(SDL_Renderer* renderer, Difficulty& difficulty);
    void shutdown();

    enum MenuItem {
        MENU_START = 0,
        MENU_OPTIONS,
        MENU_QUIT,
        MENU_COUNT
    };

    int selectedIndex = MENU_START;

    enum OptionsItem {
        DIFFICULTY_SELECT = 0,
        BACK,
        OPTIONS_COUNT
    };

    int selectedIndexOptions = DIFFICULTY_SELECT;

private:
    // Screen
    int screenW = 0;
    int screenH = 0;

    // Options flag
    bool optionsOpen = false;

    // Textures
    SDL_Texture* mainBgTexture = nullptr;
    SDL_Texture* mainLogoFgTexture = nullptr;
    SDL_Texture* mainLogoBgTexture = nullptr;
    SDL_Texture* startTexture = nullptr;
    SDL_Texture* optionsTexture = nullptr;
    SDL_Texture* difficultyTexture = nullptr;
    SDL_Texture* easyTexture = nullptr;
    SDL_Texture* mediumTexture = nullptr;
    SDL_Texture* hardTexture = nullptr;
    SDL_Texture* backTexture = nullptr;
    SDL_Texture* quitTexture = nullptr;
    SDL_Texture* cursorTexture = nullptr;

    // Layout
    SDL_Rect mainBgRect{};
    SDL_Rect mainLogoRect{};
    SDL_Rect startRect{};
    SDL_Rect optionsRect{};
    SDL_Rect difficultyRect{};
    SDL_Rect easyRect{};
    SDL_Rect mediumRect{};
    SDL_Rect hardRect{};
    SDL_Rect backRect{};
    SDL_Rect quitRect{};
    SDL_Rect cursorRect{};
    SDL_Rect menuRects[MENU_COUNT]{};
    SDL_Rect optionRects[OPTIONS_COUNT]{};

    // Cursor physics
    float cursorX = 0.0f;
    float cursorY = 0.0f;
    float cursorVX = 0.0f;
    float cursorVY = 0.0f;
    float cursorScale = 1.0f;
    float cursorScaleVel = 0.0f;
    int lastSelectedIndex = -1;

    // Ash particles
    std::vector<AshParticle> ashParticles;

    // Helpers
    void spawnAshParticle();
    void updateAndRenderAsh(SDL_Renderer* renderer, float dt);
    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path);
};

