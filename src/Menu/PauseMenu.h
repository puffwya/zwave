#pragma once
#include <SDL2/SDL.h>
#include <string>
#include "../Engine/GameState.h"

class PauseMenu {
public:
    bool init(SDL_Renderer* renderer, int screenW, int screenH);
    void handleInput(const SDL_Event& e, GameState& gs, bool& running);
    void updateCursor();
    void activateSelected(GameState& gs, bool& running);
    void render(SDL_Renderer* renderer);
    void shutdown();

    enum MenuItem {
        MENU_START = 0,
        MENU_OPTIONS,
        MENU_QUIT,
        MENU_COUNT
    };

    int selectedIndex = MENU_START;

private:
    // Screen
    int screenW = 0;
    int screenH = 0;

    // Textures
    SDL_Texture* mainBgTexture = nullptr;
    SDL_Texture* mainLogoFgTexture = nullptr;
    SDL_Texture* mainLogoBgTexture = nullptr;
    SDL_Texture* startTexture = nullptr;
    SDL_Texture* optionsTexture = nullptr;
    SDL_Texture* quitTexture = nullptr;
    SDL_Texture* cursorTexture = nullptr;

    // Layout
    SDL_Rect mainBgRect{};
    SDL_Rect mainLogoRect{};
    SDL_Rect startRect{};
    SDL_Rect optionsRect{};
    SDL_Rect quitRect{};
    SDL_Rect cursorRect{};
    SDL_Rect menuRects[MENU_COUNT]{};

    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path);
};

