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

    bool startedMusic = false;

private:
    int width = 0;
    int height = 0;

    float elapsedTime = 0.0f;
    const float duration = 8.0f;
};

