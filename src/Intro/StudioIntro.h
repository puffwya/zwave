#pragma once
#include <SDL2/SDL.h>
#include <string>
#include "../Engine/GameState.h"

class StudioIntro {
public:
    bool init(SDL_Renderer* renderer, int screenW, int screenH);
    void start();
    void handleEvent(const SDL_Event& e, GameState& gameState);
    void update(float dt, GameState& gameState);
    void render(SDL_Renderer* renderer);
    void cleanup();

private:
    SDL_Texture* logoTexture = nullptr;
    SDL_Rect logoRect{};

    float timer = 0.0f;
    float alpha = 0.0f;
    bool started = false;

    const float FADE_IN_TIME  = 2.0f;
    const float HOLD_TIME     = 5.0f;
    const float FADE_OUT_TIME = 3.0f;

    enum class Phase {
        FadeIn,
        Hold,
        FadeOut
    } phase = Phase::FadeIn;

    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path);
};

