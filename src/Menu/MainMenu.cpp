#include "MainMenu.h"
#include <SDL2/SDL_image.h>
#include <iostream>

void MainMenu::spawnAshParticle()
{
    AshParticle p;

    // Spawn around the main logo
    p.x = mainLogoRect.x + rand() % mainLogoRect.w;
    p.y = mainLogoRect.y + (rand() % (mainLogoRect.h / 2));

    // Randomize size
    p.size = 2.5f + (rand() % 100) / 100.0f * 2.5f;

    // Slow upward drift
    p.vx = ((rand() % 100) / 100.0f - 0.5f) * 4.0f;
    p.vy = -2.0f - (rand() % 2);

    p.life = 0.0f;
    p.maxLife = 6.5f + (rand() % 100) / 100.0f;

    ashParticles.push_back(p);
}

void MainMenu::updateAndRenderAsh(SDL_Renderer* renderer, float dt)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = ashParticles.size() - 1; i >= 0; --i) {
        AshParticle& p = ashParticles[i];

        p.life += dt;
        if (p.life >= p.maxLife) {
            ashParticles.erase(ashParticles.begin() + i);
            continue;
        }

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        float alpha = 1.0f - (p.life / p.maxLife);
        Uint8 a = (Uint8)(alpha * 180);

        float lifeT = p.life / p.maxLife;

        // Slight shrink as it fades
        float size = p.size * (1.0f - lifeT * 0.3f);

        SDL_SetRenderDrawColor(renderer, 255, 183, 11, a);
        SDL_Rect r = {(int)(p.x - size * 0.5f), (int)(p.y - size * 0.5f), (int)size, (int)size};
        SDL_RenderFillRect(renderer, &r);
    }
}

bool MainMenu::init(SDL_Renderer* renderer, int screenW, int screenH) {
    const float buttonWidth = screenW * 0.20f;

    const int leftPadding = (int)(screenW * 0.05f); // 5% from left
    const int topPadding  = (int)(screenH * 0.08f);
    const int buttonGap   = (int)(screenH * 0.04f);

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

    surface = IMG_Load("assets/pixDigit/main_logo_fg.png");
    if (!surface) {
        std::cerr << "Failed to load main logo fg: " << IMG_GetError() << std::endl;
        return false;
    }   
        
    mainLogoFgTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float logoAspect = (float)surface->h / surface->w;

    mainLogoRect.w = (int)(screenW * 0.40f);
    mainLogoRect.h = (int)(mainLogoRect.w * logoAspect);

    mainLogoRect.x = leftPadding;
    mainLogoRect.y = topPadding;
     
    SDL_FreeSurface(surface);
    
    if (!mainLogoFgTexture) {
        std::cerr << "Failed to create main logo texture\n";
        return false;
    }

    // Main Logo Background

    surface = IMG_Load("assets/pixDigit/main_logo_bg.png");
    if (!surface) {
        std::cerr << "Failed to load main logo bg: " << IMG_GetError() << std::endl;
        return false;
    }

    mainLogoBgTexture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);
    
    if (!mainLogoBgTexture) {
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

    float startAspect = (float)surface->h / surface->w;

    startRect.w = (int)(screenW * 0.20f);
    startRect.h = (int)(startRect.w * startAspect);

    startRect.x = leftPadding + int(mainLogoRect.w * 0.25f);
    startRect.y = mainLogoRect.y + mainLogoRect.h + buttonGap;

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

    float optionsAspect = (float)surface->h / surface->w;

    optionsRect.w = startRect.w;
    optionsRect.h = (int)(optionsRect.w * optionsAspect);

    optionsRect.x = leftPadding + int(mainLogoRect.w * 0.25f);
    optionsRect.y = startRect.y + startRect.h + buttonGap;

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

    float quitAspect = (float)surface->h / surface->w;

    quitRect.w = startRect.w;
    quitRect.h = (int)(quitRect.w * quitAspect);

    quitRect.x = leftPadding + int(mainLogoRect.w * 0.25f);
    quitRect.y = optionsRect.y + optionsRect.h + buttonGap;

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

void MainMenu::handleInput(const SDL_Event& e, GameState& gs, bool& running, bool& mRunning) {
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
            activateSelected(gs, running, mRunning);
            break;
    }
}

void MainMenu::updateCursor(float dt)
{
    const SDL_Rect& target = menuRects[selectedIndex];

    // Target position (to the right of button)
    float targetX = target.x + target.w + 12.0f;
    float targetY = target.y + (target.h * 0.5f) - (cursorRect.h * 0.5f);

    // Detect selection change
    if (selectedIndex != lastSelectedIndex)
    {
        // Overshoot impulse (directional)
        float dir = (selectedIndex > lastSelectedIndex) ? 1.0f : -1.0f;
        cursorVY += dir * 180.0f;   // overshoot strength (~4–6px visually)

        // Scale pulse
        cursorScaleVel += 3.5f;

        lastSelectedIndex = selectedIndex;
    }

    // Spring-like motion (lerp + inertia)
    float stiffness = 70.0f;   // responsiveness
    float damping   = 12.0f;   // prevents oscillation

    cursorVX += (targetX - cursorX) * stiffness * dt;
    cursorVY += (targetY - cursorY) * stiffness * dt;

    cursorVX *= expf(-damping * dt);
    cursorVY *= expf(-damping * dt);

    cursorX += cursorVX * dt;
    cursorY += cursorVY * dt;

    // Scale pulse spring
    float scaleStiffness = 22.0f;
    float scaleDamping   = 14.0f;

    cursorScaleVel += (1.0f - cursorScale) * scaleStiffness * dt;
    cursorScaleVel *= expf(-scaleDamping * dt);
    cursorScale += cursorScaleVel * dt;

    // Apply to rect
    cursorRect.x = (int)cursorX;
    cursorRect.y = (int)cursorY;
}

void MainMenu::activateSelected(GameState& gs, bool& running, bool& mRunning) {
    if (selectedIndex == MENU_START) {
        gs = GameState::Playing;
    }
    else if (selectedIndex == MENU_OPTIONS) {
        gs = GameState::MainMenu;
    }
    else if (selectedIndex == MENU_QUIT) {
        running = false;
        mRunning = false;
    }
}

void MainMenu::render(SDL_Renderer* renderer)
{
    SDL_Rect uiPanelRect;

    uiPanelRect.x = 0;
    uiPanelRect.y = 0;

    uiPanelRect.w = mainLogoRect.w + (int)(mainBgRect.w * 0.05f);;
    uiPanelRect.h = mainBgRect.h;


    SDL_RenderClear(renderer);

    float t = SDL_GetTicks() * 0.001f;
    float dt = 1.0f / 60.0f;

    // Fire glow pulse
    float pulse = 0.75f + 0.25f * sinf(t * 2.5f);
    pulse = fmaxf(0.6f, fminf(1.2f, pulse));
    Uint8 glow = (Uint8)(255 * pulse);

    SDL_SetTextureBlendMode(mainLogoBgTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(mainLogoBgTexture, glow, glow, glow);
    SDL_SetTextureAlphaMod(mainLogoBgTexture, 245);

    SDL_RenderCopy(renderer, mainBgTexture, nullptr, &mainBgRect);

    // Ash particles
    if (rand() % 100 < 3) {
        spawnAshParticle();
    }
    updateAndRenderAsh(renderer, dt);

    // UI backing plate
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 5, 5, 5, 170);
    SDL_RenderFillRect(renderer, &uiPanelRect);

    // Soft fade at right edge of UI backing plate
    const int fadeWidth = 40;

    for (int i = 0; i < fadeWidth; i++)
    {
        float t = (float)i / fadeWidth;
        Uint8 alpha = (Uint8)(170 * (1.0f - t));

        SDL_SetRenderDrawColor(renderer, 5, 5, 5, alpha);

        SDL_Rect strip = {
            uiPanelRect.x + uiPanelRect.w + i,
            uiPanelRect.y,
            1,
            uiPanelRect.h
        };

        SDL_RenderFillRect(renderer, &strip);
    }

    // Logo background
    SDL_RenderCopy(renderer, mainLogoBgTexture, nullptr, &mainLogoRect);

    // Logo foreground
    SDL_RenderCopy(renderer, mainLogoFgTexture, nullptr, &mainLogoRect);

    // Menu UI
    SDL_RenderCopy(renderer, startTexture, nullptr, &startRect);
    SDL_RenderCopy(renderer, optionsTexture, nullptr, &optionsRect);
    SDL_RenderCopy(renderer, quitTexture, nullptr, &quitRect);

    // Menu Cursor
    SDL_Rect renderRect = cursorRect;
    renderRect.w = (int)(cursorRect.w * cursorScale);
    renderRect.h = (int)(cursorRect.h * cursorScale);

    // Keep scale centered
    renderRect.x -= (renderRect.w - cursorRect.w) / 2;
    renderRect.y -= (renderRect.h - cursorRect.h) / 2;

    SDL_RenderCopy(renderer, cursorTexture, nullptr, &renderRect);
}

