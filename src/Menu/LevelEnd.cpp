#include <SDL2/SDL_image.h>
#include <iostream>
#include "LevelEnd.h"

bool LevelEnd::init(SDL_Renderer* renderer, int screenW, int screenH) {
    width = screenW;
    height = screenH;

    SDL_Surface* surface = nullptr;

    // SECTOR png
    surface = IMG_Load("Assets/pixDigit/lvlEndSector.png");
    if (!surface) {
        std::cerr << "Failed to load sector png: " << IMG_GetError() << std::endl;
        return false;
    }

    sectorTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float sectorAspect = (float)surface->h / surface->w;

    sectorRect.w = (int)(screenW * 0.45f);
    sectorRect.h = (int)(sectorRect.w * sectorAspect);

    sectorRect.y = (screenH / 2) - sectorRect.h;

    // Start off-screen left
    sectorRect.x = -sectorRect.w;

    SDL_FreeSurface(surface);

    if (!sectorTexture) {
        std::cerr << "Failed to create sector texture\n";
        return false;
    }

    // PURGED png
    surface = IMG_Load("Assets/pixDigit/lvlEndPurged.png");
    if (!surface) {
        std::cerr << "Failed to load purged png: " << IMG_GetError() << std::endl;
        return false;
    }

    purgedTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float purgedAspect = (float)surface->h / surface->w;

    purgedRect.w = sectorRect.w;
    purgedRect.h = (int)(purgedRect.w * purgedAspect);

    purgedRect.y = (screenH / 2) + 10;

    // Start off-screen right
    purgedRect.x = screenW;

    SDL_FreeSurface(surface);

    if (!purgedTexture) {
        std::cerr << "Failed to create purged texture\n";
        return false;
    }

    // Target center positions
    sectorTargetX = (screenW / 2) - sectorRect.w;
    purgedTargetX = (screenW / 2);

    return true;
}

void LevelEnd::handleInput(SDL_Event& e, GameState& gameState, bool& running) {
    if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_RETURN) {
            gameState = GameState::MainMenu;
        }
        if (e.key.keysym.sym == SDLK_ESCAPE) {
            running = false;
        }
    }
}

void LevelEnd::update(float dt, GameState& gameState, AudioManager& audio)
{
    elapsedTime += dt;

    // Auto Return Timer
    if (elapsedTime >= returnDelay) {
        gameState = GameState::MainMenu;
        elapsedTime = 0.0f;
        return;
    }

    // Slide Start
    if (elapsedTime >= slideDelay && !collided)
    {
        if (!sliding) {
            sliding = true;
            audio.playSFX("lvlEnd_wordsCollide");
        }
    }

    // Sliding Logic
    if (sliding && !collided)
    {
        sectorRect.x += (int)(slideSpeed * dt);
        purgedRect.x -= (int)(slideSpeed * dt);

        if (sectorRect.x >= sectorTargetX)
        {
            sectorRect.x = sectorTargetX;
            purgedRect.x = purgedTargetX;

            collided = true;
            impactTime = impactDuration;
        }
    }

    // Impact Timer
    if (collided && impactTime > 0.0f)
    {
        impactTime -= dt;
    }

    // Ember Spawning
    if (embers.size() < maxEmbers)
    {
        Ember e;

        e.x = (float)(rand() % width);
        e.y = (float)(height + rand() % 30);

        e.speed = 20.0f + (rand() % 30);      // slower
        e.vx = -10.0f + (rand() % 20);        // gentle sideways drift

        e.size = 1.5f + (rand() % 2);         // smaller

        e.life = 4.0f + ((rand() % 100) / 100.0f);

        embers.push_back(e);
    }

    for (auto& e : embers)
    {
        e.y -= e.speed * dt;
        e.x += e.vx * dt;

        e.life -= dt;
    }

    // Remove dead embers
    embers.erase(
        std::remove_if(embers.begin(), embers.end(),
            [](Ember& e) { return e.life <= 0.0f; }),
        embers.end());
}

void LevelEnd::render(SDL_Renderer* renderer) {

    int shakeX = 0;
    int shakeY = 0;

    if (impactTime > 0.0f)
    {
        shakeX = rand() % 20 - 10;
        shakeY = rand() % 20 - 10;
    }

    // Black background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Bottom Glow
    int glowHeight = height / 2 + 200;

    for (int i = 0; i < glowHeight; i++)
    {
        float t = (float)i / glowHeight;

        // Much darker and more subtle
        Uint8 r = (Uint8)(120 * (1.0f - t));
        Uint8 g = (Uint8)(25 * (1.0f - t));
        Uint8 b = (Uint8)(10 * (1.0f - t));

        Uint8 alpha = (Uint8)(90 * (1.0f - t)); // lower alpha

        SDL_SetRenderDrawColor(renderer, r, g, b, alpha);

        SDL_Rect line = { 0, height - i, width, 1 };
        SDL_RenderFillRect(renderer, &line);
    }

    // Ember Rendering
    for (const auto& e : embers)
    {
        float lifeRatio = e.life / 3.0f;
        if (lifeRatio < 0) lifeRatio = 0;

        Uint8 alpha = (Uint8)(200 * lifeRatio);

        // darker ember tones
        Uint8 r = (Uint8)(200 * lifeRatio);
        Uint8 g = (Uint8)(80 * lifeRatio);
        Uint8 b = 20;

        SDL_SetRenderDrawColor(renderer, r, g, b, alpha);

        SDL_Rect emberRect =
        {
            (int)e.x,
            (int)e.y,
            (int)e.size,
            (int)e.size
        };

        SDL_RenderFillRect(renderer, &emberRect);
    }

    // Sector Purged rendering

    SDL_Rect sectorRender = sectorRect;
    SDL_Rect purgedRender = purgedRect;

    sectorRender.x += shakeX;
    sectorRender.y += shakeY;

    purgedRender.x += shakeX;
    purgedRender.y += shakeY;

    SDL_RenderCopy(renderer, sectorTexture, nullptr, &sectorRender);
    SDL_RenderCopy(renderer, purgedTexture, nullptr, &purgedRender);
}

void LevelEnd::resetAnimation()
{
    elapsedTime = 0.0f;

    sliding = false;
    collided = false;

    impactTime = 0.0f;

    // Reset positions
    sectorRect.x = -sectorRect.w;
    purgedRect.x = width;

    startedMusic = false;
}

void LevelEnd::cleanup() {
    if (sectorTexture) SDL_DestroyTexture(sectorTexture);
    if (purgedTexture) SDL_DestroyTexture(purgedTexture);
}

