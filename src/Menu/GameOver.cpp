#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>
#include "GameOver.h"

SDL_Texture* GameOver::loadTexture(SDL_Renderer* renderer, const std::string& path)
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

bool GameOver::init(SDL_Renderer* renderer, int screenW, int screenH)
{
    width = screenW;
    height = screenH;

    // Load textures
    playerDiedTexture = loadTexture(renderer, "Assets/pixDigit/playerDied.png");
    bloodOverlayTexture = loadTexture(renderer, "Assets/pixDigit/bloodOverlay.png");

    if (!playerDiedTexture || !bloodOverlayTexture)
        return false;

    // Setup playerDied layout
    int texW, texH;
    SDL_QueryTexture(playerDiedTexture, nullptr, nullptr, &texW, &texH);

    float aspect = static_cast<float>(texH) / texW;

    playerDiedRect.w = static_cast<int>(screenW * 0.45f);
    playerDiedRect.h = static_cast<int>(playerDiedRect.w * aspect);

    playerDiedRect.x = (screenW / 2) - (playerDiedRect.w / 2);
    playerDiedRect.y = (screenH / 2) - playerDiedRect.h;

    // Blood overlay fills screen
    bloodOverlayRect = { 0, 0, screenW, screenH };

    SDL_SetTextureBlendMode(bloodOverlayTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(playerDiedTexture, SDL_BLENDMODE_BLEND);

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
        switch (e.key.keysym.sym)
        {
            case SDLK_RETURN:
                gameState = GameState::MainMenu;
                break;

            case SDLK_ESCAPE:
                running = false;
                break;

            default:
                break;
        }
    }
}

void GameOver::update(float dt, GameState& gameState)
{
    elapsedTime += dt;

    if (elapsedTime >= duration)
        gameState = GameState::MainMenu;
}

void GameOver::render(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Black fade
    float blackAlpha = 1.0f;

    if (elapsedTime < blackFadeDuration)
        blackAlpha = elapsedTime / blackFadeDuration;

    blackAlpha = std::clamp(blackAlpha, 0.0f, 1.0f);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0,
        static_cast<Uint8>(blackAlpha * 255));

    SDL_Rect fullScreen = { 0, 0, width, height };
    SDL_RenderFillRect(renderer, &fullScreen);

    // Blood + Text fade
    if (elapsedTime >= textFadeStart)
    {
        float fadeTime = elapsedTime - textFadeStart;
        float alpha = fadeTime < textFadeDuration
                        ? fadeTime / textFadeDuration
                        : 1.0f;

        alpha = std::clamp(alpha, 0.0f, 1.0f);
        Uint8 alphaByte = static_cast<Uint8>(alpha * 255);

        SDL_SetTextureAlphaMod(bloodOverlayTexture, alphaByte);
        SDL_RenderCopy(renderer, bloodOverlayTexture, nullptr, &bloodOverlayRect);

        SDL_SetTextureAlphaMod(playerDiedTexture, alphaByte);
        SDL_RenderCopy(renderer, playerDiedTexture, nullptr, &playerDiedRect);
    }
}

void GameOver::cleanup()
{
    SDL_DestroyTexture(playerDiedTexture);
    SDL_DestroyTexture(bloodOverlayTexture);
}

