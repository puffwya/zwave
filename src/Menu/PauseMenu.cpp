#include "PauseMenu.h"
#include <SDL2/SDL_image.h>
#include <iostream>

bool PauseMenu::init(SDL_Renderer* renderer, int screenW, int screenH) {
    const float buttonWidth = screenW * 0.20f;
    const float buttonGap = screenH * 0.06f;

    // Main Menu Background

    SDL_Surface* surface = IMG_Load("assets/pixDigit/mainBgPix.png");
    if (!surface) {
        std::cerr << "Failed to load main background img: " << IMG_GetError() << std::endl;
        return false;
    }
    
    mainBgTexture = SDL_CreateTextureFromSurface(renderer, surface);
    
    float mainBgAspect  = (float)surface->h / surface->w;
    
    mainBgRect.w = screenW;
    mainBgRect.h = screenH;
    
    mainBgRect.x = 0;
    mainBgRect.y = 0;
    
    SDL_FreeSurface(surface);
    
    if (!mainBgTexture) {
        std::cerr << "Failed to create main background texture\n";
        return false;
    }

    // Main Logo

    surface = IMG_Load("assets/pixDigit/mainLogoPix.png");
    if (!surface) {
        std::cerr << "Failed to load main logo: " << IMG_GetError() << std::endl;
        return false;
    }   
        
    mainLogoTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float logoAspect  = (float)surface->h / surface->w;

    mainLogoRect.w = (int)(screenW * 0.40f);
    mainLogoRect.h = (int)(mainLogoRect.w * logoAspect);

    mainLogoRect.x = (screenW - mainLogoRect.w) / 2;
    mainLogoRect.y = (int)(screenH * 0.15f);
     
    SDL_FreeSurface(surface);
    
    if (!mainLogoTexture) {
        std::cerr << "Failed to create main logo texture\n";
        return false;
    }

    // Start Button

    surface = IMG_Load("assets/pixDigit/startPix.png");
    if (!surface) {
        std::cerr << "Failed to load start button: " << IMG_GetError() << std::endl;
        return false;
    }

    startTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float startAspect  = (float)surface->h / surface->w;

    startRect.w = (int)(screenW * 0.20f);
    startRect.h = (int)(startRect.w * startAspect);

    startRect.x = (screenW - startRect.w) / 2;
    startRect.y = (int)(screenH * 0.45f);

    menuRects[MENU_START] = startRect;

    SDL_FreeSurface(surface);

    if (!startTexture) {
        std::cerr << "Failed to create start texture\n";
        return false;
    }

    // Options Button

    surface = IMG_Load("assets/pixDigit/optionsPix.png");
    if (!surface) {
        std::cerr << "Failed to load options button: " << IMG_GetError() << std::endl;
        return false;
    }

    optionsTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float optionsAspect  = (float)surface->h / surface->w;

    optionsRect.w = startRect.w;
    optionsRect.h = (int)(optionsRect.w * optionsAspect);

    optionsRect.x = startRect.x;
    optionsRect.y = startRect.y + buttonGap;

    menuRects[MENU_OPTIONS] = optionsRect;

    SDL_FreeSurface(surface);

    if (!optionsTexture) {
        std::cerr << "Failed to create options texture\n";
        return false;
    }

    // Quit Button

    surface = IMG_Load("assets/pixDigit/quitPix.png");
    if (!surface) {
        std::cerr << "Failed to load quit button: " << IMG_GetError() << std::endl;
        return false;
    }

    quitTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float quitAspect  = (float)surface->h / surface->w;

    quitRect.w = startRect.w;
    quitRect.h = (int)(quitRect.w * quitAspect);

    quitRect.x = startRect.x;
    quitRect.y = optionsRect.y + buttonGap;

    menuRects[MENU_QUIT] = quitRect;

    SDL_FreeSurface(surface);

    if (!quitTexture) {
        std::cerr << "Failed to create quit texture\n";
        return false;
    }

    // Select Cursor
    
    surface = IMG_Load("assets/pixDigit/cursorPix.png");
    if (!surface) {
        std::cerr << "Failed to load cursor: " << IMG_GetError() << std::endl;
        return false;
    }
 
    cursorTexture = SDL_CreateTextureFromSurface(renderer, surface);
     
    float cursorAspect  = (float)surface->h / surface->w;
    
    cursorRect.w = (int)(screenW * 0.05f);
    cursorRect.h = (int)(cursorRect.w * cursorAspect);
    
    cursorRect.x = menuRects[selectedIndex].x + 12;
    cursorRect.y = menuRects[selectedIndex].y;

    SDL_FreeSurface(surface);
    
    if (!cursorTexture) {
        std::cerr << "Failed to create cursor texture\n";
        return false;
    }

    return true;
}

void PauseMenu::handleInput(const SDL_Event& e, GameState& gs, bool& running) {
    if (e.type != SDL_KEYDOWN) return;

    switch (e.key.keysym.sym) {
        case SDLK_UP:
            selectedIndex--;
            if (selectedIndex < 0)
                selectedIndex = MENU_COUNT - 1;
            break;

        case SDLK_DOWN:
            selectedIndex++;
            if (selectedIndex >= MENU_COUNT)
                selectedIndex = 0;
            break;

        case SDLK_RETURN:
            activateSelected(gs, running);
            break;
    }
}

void PauseMenu::updateCursor() {
    const SDL_Rect& target = menuRects[selectedIndex];

    cursorRect.x = target.x + target.w + 12; // to the RIGHT of button
    cursorRect.y = target.y + (target.h / 2) - (cursorRect.h / 2);
}

void PauseMenu::activateSelected(GameState& gs, bool& running) {
    if (selectedIndex == MENU_START) {
        gs = GameState::Playing;
    }
    else if (selectedIndex == MENU_OPTIONS) {
        gs = GameState::MainMenu;
    }
    else if (selectedIndex == MENU_QUIT) {
        gs = GameState::MainMenu;
        running = false;
    }
}

void PauseMenu::render(SDL_Renderer* renderer) {
    //SDL_RenderClear(renderer);

    //SDL_RenderCopy(renderer, mainBgTexture, nullptr, &mainBgRect);
    SDL_RenderCopy(renderer, mainLogoTexture, nullptr, &mainLogoRect);
    SDL_RenderCopy(renderer, startTexture, nullptr, &startRect);
    SDL_RenderCopy(renderer, optionsTexture, nullptr, &optionsRect);
    SDL_RenderCopy(renderer, quitTexture, nullptr, &quitRect);
    SDL_RenderCopy(renderer, cursorTexture, nullptr, &cursorRect);
}
