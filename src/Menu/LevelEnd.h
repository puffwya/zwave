#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../Engine/GameState.h"
#include "../audio/AudioManager.h"
#include "../Engine/Player.h"
#include "../Engine/EnemyManager.h"

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
    void render(SDL_Renderer* renderer, Player& player, EnemyManager& em);
    void resetAnimation();
    void cleanup();

private:
    int width = 0;
    int height = 0;

    float enemiesTime = 0.2f;
    float shotsTime   = 0.5f;
    float hitsTime    = 0.8f;
    float accuracyTime= 1.1f;
    float timeTime    = 1.4f;

    float fadeDuration = 0.4f;

    // Digit tex
    SDL_Texture* digitTextures[11]{};
    int digitW = 0;
    int digitH = 0;

    // Text textures
    SDL_Texture* sectorTexture = nullptr;
    SDL_Texture* purgedTexture = nullptr;
    SDL_Texture* enemiesTermTexture;
    SDL_Texture* shotsFiredTexture;
    SDL_Texture* directHitsTexture;
    SDL_Texture* accuracyTexture;
    SDL_Texture* timeElapsedTexture;

    // Rects
    SDL_Rect sectorRect{};
    SDL_Rect purgedRect{};
    SDL_Rect enemiesTermRect;
    SDL_Rect shotsFiredRect;
    SDL_Rect directHitsRect;
    SDL_Rect accuracyRect;
    SDL_Rect timeElapsedRect;

    // Target positions
    int sectorTargetX = 0;
    int purgedTargetX = 0;

    float sectorYFloat;
    float purgedYFloat;

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

    bool slidingUp = false;
    float slideUpSpeed = 400.0f;
    int finalTopY;                 // target Y position

    bool reportActive = false;
    float reportTimer = 0.0f;

    float elapsedTime = 0.0f;
    const float returnDelay = 55.0f;  // Return to main menu after 55 seconds (song length)

    void renderStat(SDL_Renderer* renderer,
                          SDL_Texture* texture,
                          SDL_Rect rect,
                          float timer,
                          float appearTime, int value);

    void renderStatNumbers(SDL_Renderer* renderer,
                       SDL_Rect labelRect,
                       int value,
                       SDL_Texture* digitTextures[10],
                       float alpha,
                       int spacing = 2);
};

