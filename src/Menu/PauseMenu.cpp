#include "PauseMenu.h"
#include <SDL2/SDL_image.h>
#include <iostream>

// Texture Loader helper
SDL_Texture* PauseMenu::loadTexture(SDL_Renderer* renderer,
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

// Init
bool PauseMenu::init(SDL_Renderer* renderer, int w, int h)
{
    screenW = w;
    screenH = h;

    const float buttonWidth = screenW * 0.20f;
    const float buttonGap = screenH * 0.08f;

    // Load textures
    mainBgTexture = loadTexture(renderer, "assets/pixDigit/mainBgPix.png");
    mainLogoFgTexture = loadTexture(renderer, "assets/pixDigit/main_logo_fg.png");
    mainLogoBgTexture = loadTexture(renderer, "assets/pixDigit/main_logo_bg.png");
    startTexture = loadTexture(renderer, "assets/pixDigit/startPix.png");
    optionsTexture = loadTexture(renderer, "assets/pixDigit/optionsPix.png");
    quitTexture = loadTexture(renderer, "assets/pixDigit/quitPix.png");
    cursorTexture = loadTexture(renderer, "assets/pixDigit/cursorPix.png");

    if (!mainLogoFgTexture || !mainLogoBgTexture ||
        !startTexture || !optionsTexture ||
        !quitTexture || !cursorTexture)
        return false;

    mainBgRect = { 0, 0, screenW, screenH };

    // Logo
    int texW, texH;
    SDL_QueryTexture(mainLogoFgTexture, nullptr, nullptr, &texW, &texH);
    float logoAspect = static_cast<float>(texH) / texW;

    mainLogoRect.w = static_cast<int>(screenW * 0.40f);
    mainLogoRect.h = static_cast<int>(mainLogoRect.w * logoAspect);
    mainLogoRect.x = (screenW - mainLogoRect.w) / 2;
    mainLogoRect.y = static_cast<int>(screenH * 0.15f);

    // Buttons
    auto setupButton = [&](SDL_Texture* tex, SDL_Rect& rect, int y)
    {
        int wTex, hTex;
        SDL_QueryTexture(tex, nullptr, nullptr, &wTex, &hTex);
        float aspect = static_cast<float>(hTex) / wTex;

        rect.w = static_cast<int>(buttonWidth);
        rect.h = static_cast<int>(rect.w * aspect);
        rect.x = (screenW - rect.w) / 2;
        rect.y = y;
    };

    int startY = static_cast<int>(screenH * 0.45f);

    setupButton(startTexture, startRect, startY);
    setupButton(optionsTexture, optionsRect,
                startRect.y + startRect.h + buttonGap);
    setupButton(quitTexture, quitRect,
                optionsRect.y + optionsRect.h + buttonGap);

    menuRects[MENU_START] = startRect;
    menuRects[MENU_OPTIONS] = optionsRect;
    menuRects[MENU_QUIT] = quitRect;

    // Cursor
    SDL_QueryTexture(cursorTexture, nullptr, nullptr, &texW, &texH);
    float cursorAspect = static_cast<float>(texH) / texW;

    cursorRect.w = static_cast<int>(screenW * 0.05f);
    cursorRect.h = static_cast<int>(cursorRect.w * cursorAspect);

    updateCursor();

    return true;
}

// Input
void PauseMenu::handleInput(const SDL_Event& e,
                            GameState& gs,
                            bool& running)
{
    if (e.type != SDL_KEYDOWN) return;

    switch (e.key.keysym.sym)
    {
        case SDLK_UP:
            selectedIndex =
                (selectedIndex - 1 + MENU_COUNT) % MENU_COUNT;
            break;

        case SDLK_DOWN:
            selectedIndex =
                (selectedIndex + 1) % MENU_COUNT;
            break;

        case SDLK_RETURN:
            activateSelected(gs, running);
            break;

        default:
            break;
    }

    updateCursor();
}

// Cursor Position
void PauseMenu::updateCursor()
{
    const SDL_Rect& target = menuRects[selectedIndex];

    cursorRect.x = target.x + target.w + 12;
    cursorRect.y = target.y + (target.h / 2)
                 - (cursorRect.h / 2);
}

// Selection Logic
void PauseMenu::activateSelected(GameState& gs,
                                 bool& running)
{
    switch (selectedIndex)
    {
        case MENU_START:
            gs = GameState::Playing;
            break;

        case MENU_OPTIONS:
            gs = GameState::MainMenu;
            break;

        case MENU_QUIT:
            gs = GameState::MainMenu;
            running = false;
            break;
    }
}

// Render
void PauseMenu::render(SDL_Renderer* renderer)
{
    // NOTE:
    // We intentionally DO NOT clear or draw background here.
    // Pause menu overlays the current gameplay frame.

    SDL_RenderCopy(renderer, mainLogoFgTexture, nullptr, &mainLogoRect);
    SDL_RenderCopy(renderer, mainLogoBgTexture, nullptr, &mainLogoRect);

    SDL_RenderCopy(renderer, startTexture, nullptr, &startRect);
    SDL_RenderCopy(renderer, optionsTexture, nullptr, &optionsRect);
    SDL_RenderCopy(renderer, quitTexture, nullptr, &quitRect);

    SDL_RenderCopy(renderer, cursorTexture, nullptr, &cursorRect);
}

// Cleanup
void PauseMenu::shutdown()
{
    SDL_DestroyTexture(mainBgTexture);
    SDL_DestroyTexture(mainLogoFgTexture);
    SDL_DestroyTexture(mainLogoBgTexture);
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(optionsTexture);
    SDL_DestroyTexture(quitTexture);
    SDL_DestroyTexture(cursorTexture);
}

