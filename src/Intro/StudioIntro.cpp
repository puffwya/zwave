#include "StudioIntro.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>

// Texture Loader helper
SDL_Texture* StudioIntro::loadTexture(SDL_Renderer* renderer,
                                      const std::string& path)
{
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::cerr << "Failed to load: " << path
                  << " | SDL_image error: " << IMG_GetError() << "\n";
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture)
        std::cerr << "Failed to create texture from: " << path << "\n";

    return texture;
}

// Start Intro
void StudioIntro::start()
{
    timer = 0.0f;
    alpha = 0.0f;
    phase = Phase::FadeIn;
    started = true;
}

// Init
bool StudioIntro::init(SDL_Renderer* renderer,
                       int screenW,
                       int screenH)
{
    logoTexture = loadTexture(renderer, "Assets/StudioLogo.png");
    if (!logoTexture)
        return false;

    int texW, texH;
    SDL_QueryTexture(logoTexture, nullptr, nullptr, &texW, &texH);

    const float scale = 1.15f;

    logoRect.w = static_cast<int>(texW * scale);
    logoRect.h = static_cast<int>(texH * scale);
    logoRect.x = (screenW - logoRect.w) / 2;
    logoRect.y = (screenH - logoRect.h) / 2;

    SDL_SetTextureBlendMode(logoTexture, SDL_BLENDMODE_BLEND);

    return true;
}

// Skip Handling
void StudioIntro::handleEvent(const SDL_Event& e,
                              GameState& gameState)
{
    if (e.type == SDL_KEYDOWN ||
        e.type == SDL_MOUSEBUTTONDOWN)
    {
        gameState = GameState::MainMenu;
    }
}

// Update Fade Logic
void StudioIntro::update(float dt,
                         GameState& gameState)
{
    timer += dt;

    float totalTime = FADE_IN_TIME + HOLD_TIME + FADE_OUT_TIME;

    if (timer < FADE_IN_TIME)
    {
        alpha = (timer / FADE_IN_TIME) * 255.0f;
    }
    else if (timer < FADE_IN_TIME + HOLD_TIME)
    {
        alpha = 255.0f;
    }
    else if (timer < totalTime)
    {
        float t = timer - (FADE_IN_TIME + HOLD_TIME);
        alpha = 255.0f * (1.0f - (t / FADE_OUT_TIME));
    }
    else
    {
        gameState = GameState::MainMenu;
    }

    alpha = std::clamp(alpha, 0.0f, 255.0f);
}

// Render
void StudioIntro::render(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // White background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    if (!logoTexture)
        return;

    SDL_SetTextureAlphaMod(logoTexture,
                           static_cast<Uint8>(alpha));

    SDL_RenderCopy(renderer, logoTexture,
                   nullptr, &logoRect);
}

// Cleanup
void StudioIntro::cleanup()
{
    if (logoTexture)
    {
        SDL_DestroyTexture(logoTexture);
        logoTexture = nullptr;
    }
}

