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
    float vx;
    float speed;
    float size;
    float life;
};

class LevelEnd {
public:
    bool startedMusic = false;   // Used externally

    bool init(SDL_Renderer* renderer, int screenW, int screenH);
    void handleInput(SDL_Event& e, GameState& gameState, bool& running);
    void update(float dt, GameState& gameState, AudioManager& audio);
    void render(SDL_Renderer* renderer, Player& player, EnemyManager& em);
    void resetAnimation();
    void cleanup();

private:
    static constexpr int DIGIT_COUNT = 10;

    // Screen dimensions
    int width = 0;
    int height = 0;

    // Animation timing
    float elapsedTime = 0.0f;
    const float returnDelay = 55.0f;

    float slideDelay = 0.5f;
    float slideSpeed = 1800.0f;

    float impactTime = 0.0f;
    float impactDuration = 0.2f;

    bool sliding = false;
    bool collided = false;
    bool slidingUp = false;

    float slideUpSpeed = 400.0f;
    int finalTopY = 0;

    // Stat reveal timing
    float enemiesTime  = 0.2f;
    float shotsTime    = 0.5f;
    float hitsTime     = 0.8f;
    float accuracyTime = 1.1f;
    float timeTime     = 1.4f;

    float fadeDuration = 0.4f;

    bool reportActive = false;
    float reportTimer = 0.0f;

    // Digit textures
    SDL_Texture* digitTextures[DIGIT_COUNT]{};

    // Title textures
    SDL_Texture* sectorTexture = nullptr;
    SDL_Texture* purgedTexture = nullptr;

    SDL_Texture* enemiesTermTexture = nullptr;
    SDL_Texture* shotsFiredTexture = nullptr;
    SDL_Texture* directHitsTexture = nullptr;
    SDL_Texture* accuracyTexture = nullptr;
    SDL_Texture* timeElapsedTexture = nullptr;

    // Layout rects
    SDL_Rect sectorRect{};
    SDL_Rect purgedRect{};

    SDL_Rect enemiesTermRect{};
    SDL_Rect shotsFiredRect{};
    SDL_Rect directHitsRect{};
    SDL_Rect accuracyRect{};
    SDL_Rect timeElapsedRect{};

    int sectorTargetX = 0;
    int purgedTargetX = 0;

    float sectorYFloat = 0.0f;
    float purgedYFloat = 0.0f;

    std::vector<Ember> embers;
    int maxEmbers = 40;

    void renderStat(SDL_Renderer* renderer,
                    SDL_Texture* texture,
                    SDL_Rect rect,
                    float timer,
                    float appearTime,
                    int value);

    void renderStatNumbers(SDL_Renderer* renderer,
                           SDL_Rect labelRect,
                           int value,
                           float alpha,
                           int spacing = 2);

    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path);
};

