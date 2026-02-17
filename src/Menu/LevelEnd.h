#pragma once
#include <SDL2/SDL.h>
#include "../Engine/GameState.h"

class LevelEnd {
public:
    bool startedMusic = false;

    void init(SDL_Renderer* renderer, int screenW, int screenH);
    void handleInput(SDL_Event& e, GameState& gameState, bool& running);
    void update(float dt, GameState& gameState);
    void render(SDL_Renderer* renderer);
    void cleanup();

private:
    int width = 0;
    int height = 0;

    float elapsedTime = 0.0f;
    const float returnDelay = 55.0f;  // Return to main menu after 55 seconds (song length)
};

