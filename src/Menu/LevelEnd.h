#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../Engine/GameState.h"
#include "../audio/AudioManager.h"

struct Ember
{
    float x;
    float y;
    float vx;   // sideways drift
    float speed;
    float size;
    float life;
};

class LevelEnd {
public:
    bool startedMusic = false;

    bool init(SDL_Renderer* renderer, int screenW, int screenH);
    void handleInput(SDL_Event& e, GameState& gameState, bool& running);
    void update(float dt, GameState& gameState, AudioManager& audio);
    void render(SDL_Renderer* renderer);
    void resetAnimation();
    void cleanup();

private:
    int width = 0;
    int height = 0;

    // Text textures
    SDL_Texture* sectorTexture = nullptr;
    SDL_Texture* purgedTexture = nullptr;

    // Rects
    SDL_Rect sectorRect{};
    SDL_Rect purgedRect{};

    // Target positions
    int sectorTargetX = 0;
    int purgedTargetX = 0;

    std::vector<Ember> embers;
    int maxEmbers = 40;

    // Animation
    float slideSpeed = 1800.0f; // pixels per second
    bool hasCollided = false;

    float slideDelay = 0.5f;   // wait before sliding
    float impactTime = 0.0f;   // for shake timing
    float impactDuration = 0.2f;

    bool sliding = false;
    bool collided = false;

    float elapsedTime = 0.0f;
    const float returnDelay = 55.0f;  // Return to main menu after 55 seconds (song length)
};

