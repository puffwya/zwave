#include "GameOver.h"

bool GameOver::init(SDL_Renderer* renderer, int screenW, int screenH)
{
    width = screenW;
    height = screenH;

    // No textures yet, but keeps structure consistent
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
    // Black background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Red square centered
    int size = height / 4;

    SDL_Rect square;
    square.w = size;
    square.h = size;
    square.x = (width / 2) - (size / 2);
    square.y = (height / 2) - (size / 2);

    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &square);
}

