#include <SDL2/SDL_image.h>
#include <iostream>
#include "GameOver.h"

bool GameOver::init(SDL_Renderer* renderer, int screenW, int screenH)
{
    width = screenW;
    height = screenH;

    SDL_Surface* surface = nullptr;

    // playerDied png
    surface = IMG_Load("Assets/pixDigit/playerDied.png");
    if (!surface) {
        std::cerr << "Failed to load playerDied png: " << IMG_GetError() << std::endl;
        return false;
    }

    playerDiedTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float playerDiedAspect = (float)surface->h / surface->w;

    playerDiedRect.w = (int)(screenW * 0.45f);
    playerDiedRect.h = (int)(playerDiedRect.w * playerDiedAspect);

    playerDiedRect.y = (screenH / 2) - playerDiedRect.h;

    // Centered
    playerDiedRect.x = (screenW / 2) - (playerDiedRect.w / 2);

    SDL_FreeSurface(surface);

    if (!playerDiedTexture) {
        std::cerr << "Failed to create playerDied texture\n";
        return false;
    }

    // blood overlay png
    surface = IMG_Load("Assets/pixDigit/bloodOverlay.png");
    if (!surface) {
        std::cerr << "Failed to load bloodOverlay png: " << IMG_GetError() << std::endl;
        return false;
    }

    bloodOverlayTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!bloodOverlayTexture) {
        std::cerr << "Failed to create bloodOverlay texture\n";
        return false;
    }

    // Stretch to fill entire screen
    bloodOverlayRect.x = 0;
    bloodOverlayRect.y = 0;
    bloodOverlayRect.w = screenW;
    bloodOverlayRect.h = screenH;

    // Enable alpha blending
    SDL_SetTextureBlendMode(bloodOverlayTexture, SDL_BLENDMODE_BLEND);

    return true;
}

void GameOver::reset()
{
    elapsedTime = 0.0f;
    startedMusic = false;
}

void GameOver::handleInput(SDL_Event& e, GameState& gameState, bool& running)
{
    if (e.type == SDL_KEYDOWN)
    {
        if (e.key.keysym.sym == SDLK_RETURN)
        {
            gameState = GameState::MainMenu;
        }
        if (e.key.keysym.sym == SDLK_ESCAPE)
        {
            running = false;
        }
    }
}

void GameOver::update(float dt, GameState& gameState)
{
    elapsedTime += dt;

    if (elapsedTime >= duration)
    {
        gameState = GameState::MainMenu;
    }
}

void GameOver::render(SDL_Renderer* renderer)
{
    // BLACK FADE OVERLAY
    float blackAlpha = 1.0f;
    if (elapsedTime < blackFadeDuration)
        blackAlpha = elapsedTime / blackFadeDuration;

    Uint8 blackAlphaByte = (Uint8)(blackAlpha * 255);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); 
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, blackAlphaByte);

    SDL_Rect fullScreen = { 0, 0, width, height };
    SDL_RenderFillRect(renderer, &fullScreen);

    // BLOOD OVERLAY
    if (bloodOverlayTexture && elapsedTime >= textFadeStart)
    {
        float bloodTime = elapsedTime - textFadeStart;

        float alpha = 1.0f;
        if (bloodTime < textFadeDuration)
            alpha = bloodTime / textFadeDuration;

        Uint8 alphaByte = (Uint8)(alpha * 255);

        SDL_SetTextureBlendMode(bloodOverlayTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(bloodOverlayTexture, alphaByte);

        SDL_RenderCopy(renderer, bloodOverlayTexture, nullptr, &bloodOverlayRect);
    }

    // YOU DIED
    if (playerDiedTexture && elapsedTime >= textFadeStart)
    {
        float textTime = elapsedTime - textFadeStart;

        float alpha = 1.0f;
        if (textTime < textFadeDuration)
            alpha = textTime / textFadeDuration;

        Uint8 alphaByte = (Uint8)(alpha * 255);

        SDL_SetTextureBlendMode(playerDiedTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(playerDiedTexture, alphaByte);

        SDL_RenderCopy(renderer, playerDiedTexture, nullptr, &playerDiedRect);
    }
}

