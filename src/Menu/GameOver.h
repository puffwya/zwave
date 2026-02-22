#pragma once
#include <SDL2/SDL.h>
#include "../Engine/GameState.h"
#include "../audio/AudioManager.h"

class GameOver
{
public:
    bool init(SDL_Renderer* renderer, int screenW, int screenH);
    void handleInput(SDL_Event& e, GameState& gameState, bool& running);
    void update(float dt, GameState& gameState);
    void render(SDL_Renderer* renderer);
    void reset();
    void cleanup();

    bool startedMusic = false;   // Used externally

private:
    // Screen dimensions
    int width = 0;
    int height = 0;

    // Textures
    SDL_Texture* playerDiedTexture = nullptr;
    SDL_Texture* bloodOverlayTexture = nullptr;

    // Layout
    SDL_Rect playerDiedRect{};
    SDL_Rect bloodOverlayRect{};

    // Timing
    float elapsedTime = 0.0f;
    const float duration = 8.0f;

    float blackFadeDuration = 1.5f;
    float textFadeStart = 1.5f;
    float textFadeDuration = 1.5f;

    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path);
};

