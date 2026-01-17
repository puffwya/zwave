#include "StudioIntro.h"
#include <algorithm>
#include <SDL2/SDL_image.h>
#include <iostream>

void StudioIntro::start() {
    timer = 0.0f;
    alpha = 0.0f;
    phase = Phase::FadeIn;
    started = true;
}

bool StudioIntro::init(SDL_Renderer* renderer, int screenW, int screenH) {
    // Load WYDE WAKE studios PNG
    SDL_Surface* surface = IMG_Load("Assets/StudioLogo.png");
    if (!surface) {
        std::cerr << "Failed to load studio logo: " << IMG_GetError() << std::endl;
        return false;
    }

    logoTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!logoTexture) {
        std::cerr << "Failed to create studio logo texture\n";
        SDL_FreeSurface(surface);
        return false;
    }

    // Save original dimensions
    logoW = surface->w;
    logoH = surface->h;

    SDL_FreeSurface(surface);

    // Scale slightly larger
    const float scale = 1.15f;
    logoW = static_cast<int>(logoW * scale);
    logoH = static_cast<int>(logoH * scale);

    // Compute centered position
    logoRect.w = logoW;
    logoRect.h = logoH;
    logoRect.x = (screenW  - logoW) / 2;
    logoRect.y = (screenH - logoH) / 2;

    // Set texture to blend mode so alpha fading works
    SDL_SetTextureBlendMode(logoTexture, SDL_BLENDMODE_BLEND);

    return true;
}

void StudioIntro::handleEvent(const SDL_Event& e, GameState& gameState) {
    // Allow skipping
    if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
        gameState = GameState::MainMenu;
    }
}

void StudioIntro::update(float dt, GameState& gameState) {
    timer += dt;

    if (timer < FADE_IN_TIME) {
        // Fade in
        alpha = (timer / FADE_IN_TIME) * 255.0f;
    }
    else if (timer < FADE_IN_TIME + HOLD_TIME) {
        // Hold
        alpha = 255.0f;
    }
    else if (timer < FADE_IN_TIME + HOLD_TIME + FADE_OUT_TIME) {
        // Fade out
        float t = timer - (FADE_IN_TIME + HOLD_TIME);
        alpha = 255.0f * (1.0f - (t / FADE_OUT_TIME));
    }
    else {
        // After 10 seconds passed
        gameState = GameState::MainMenu;
    }

    alpha = std::clamp(alpha, 0.0f, 255.0f);
}

void StudioIntro::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // White background
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    if (!logoTexture) return;

    // Set fade alpha
    SDL_SetTextureAlphaMod(logoTexture, static_cast<Uint8>(alpha));

    // Draw the logo
    SDL_RenderCopy(renderer, logoTexture, nullptr, &logoRect);
}

void StudioIntro::cleanup() {
    if (logoTexture) {
        SDL_DestroyTexture(logoTexture);
        logoTexture = nullptr;
    }
}

