#include "LevelEnd.h"

void LevelEnd::init(SDL_Renderer* renderer, int screenW, int screenH) {
    width = screenW;
    height = screenH;
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

void LevelEnd::update(float dt, GameState& gameState) {
    elapsedTime += dt;

    if (elapsedTime >= returnDelay) {
        gameState = GameState::MainMenu;
        elapsedTime = 0.0f; // reset for next time
    }
}

void LevelEnd::render(SDL_Renderer* renderer) {

    // Black background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Red square in center
    SDL_Rect rect;
    rect.w = 200;
    rect.h = 200;
    rect.x = (width / 2) - (rect.w / 2);
    rect.y = (height / 2) - (rect.h / 2);

    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void LevelEnd::cleanup() {
}

