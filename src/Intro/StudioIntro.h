#pragma once
#include <SDL2/SDL.h>
#include "../Engine/GameState.h"

class StudioIntro {
public:
    // Initialize textures, sizes, positions
    bool init(SDL_Renderer* renderer, int screenW, int screenH);

    // Start the intro (reset timer, trigger jingle)
    void start();

    // Handle SDL events (optional for quitting early)
    void handleEvent(const SDL_Event& e, GameState& gameState);

    // Update fade/hold logic
    void update(float dt, GameState& gameState);

    // Render logo + background
    void render(SDL_Renderer* renderer);

    // Free textures
    void cleanup();

private:
    SDL_Texture* logoTexture = nullptr;   // Logo texture
    SDL_Rect logoRect{};                  // Rectangle for drawing logo
    int logoW = 0;                        // Original texture width
    int logoH = 0;                        // Original texture height

    // Fade/hold durations
    const float FADE_IN_TIME  = 2.0f;
    const float HOLD_TIME     = 5.0f;
    const float FADE_OUT_TIME = 3.0f;

    // Timer & state
    float timer = 0.0f;
    bool started = false;

    // Fade phase
    enum class Phase {
        FadeIn,
        Hold,
        FadeOut
    } phase = Phase::FadeIn;

    float alpha = 0.0f; // Current alpha for fade
};

