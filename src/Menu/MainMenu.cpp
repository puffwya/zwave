#include "MainMenu.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>
#include <cmath>

SDL_Texture* MainMenu::loadTexture(SDL_Renderer* renderer, const std::string& path)
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

bool MainMenu::init(SDL_Renderer* renderer, int w, int h)
{
    screenW = w;
    screenH = h;

    const int leftPadding = static_cast<int>(screenW * 0.05f);
    const int topPadding = static_cast<int>(screenH * 0.08f);
    const int buttonGap = static_cast<int>(screenH * 0.04f);
    const int difficultyOptionGap = static_cast<int>(screenW * 0.015f);

    // Load textures
    mainBgTexture = loadTexture(renderer, "assets/pixDigit/mainBgPix.png");
    mainLogoFgTexture = loadTexture(renderer, "assets/pixDigit/main_logo_fg.png");
    mainLogoBgTexture = loadTexture(renderer, "assets/pixDigit/main_logo_bg.png");
    startTexture = loadTexture(renderer, "assets/pixDigit/startPix.png");
    optionsTexture = loadTexture(renderer, "assets/pixDigit/optionsPix.png");
    difficultyTexture = loadTexture(renderer, "assets/pixDigit/difficulty.png");
    easyTexture = loadTexture(renderer, "assets/pixDigit/easy.png");
    mediumTexture = loadTexture(renderer, "assets/pixDigit/medium.png");
    hardTexture = loadTexture(renderer, "assets/pixDigit/hard.png");
    backTexture = loadTexture(renderer, "assets/pixDigit/back.png");
    quitTexture = loadTexture(renderer, "assets/pixDigit/quitPix.png");
    cursorTexture = loadTexture(renderer, "assets/pixDigit/cursorPix.png");

    if (!mainBgTexture || !mainLogoFgTexture || !mainLogoBgTexture ||
        !startTexture || !optionsTexture || !difficultyTexture ||!easyTexture || !mediumTexture || !hardTexture || !backTexture || !quitTexture || !cursorTexture)
        return false;

    mainBgRect = { 0, 0, screenW, screenH };

    // Logo layout
    int texW, texH;
    SDL_QueryTexture(mainLogoFgTexture, nullptr, nullptr, &texW, &texH);
    float logoAspect = static_cast<float>(texH) / texW;

    mainLogoRect.w = static_cast<int>(screenW * 0.40f);
    mainLogoRect.h = static_cast<int>(mainLogoRect.w * logoAspect);
    mainLogoRect.x = leftPadding;
    mainLogoRect.y = topPadding;

    // Buttons
    auto setupButton = [&](SDL_Texture* tex, SDL_Rect& rect, int y)
    {
        int wTex, hTex;
        SDL_QueryTexture(tex, nullptr, nullptr, &wTex, &hTex);
        float aspect = static_cast<float>(hTex) / wTex;

        rect.w = static_cast<int>(screenW * 0.20f);
        rect.h = static_cast<int>(rect.w * aspect);
        rect.x = leftPadding + static_cast<int>(mainLogoRect.w * 0.25f);
        rect.y = y;
    };

    setupButton(startTexture, startRect,
        mainLogoRect.y + mainLogoRect.h + buttonGap);

    setupButton(optionsTexture, optionsRect,
        startRect.y + startRect.h + buttonGap);

    setupButton(difficultyTexture, difficultyRect,
        mainLogoRect.y + mainLogoRect.h + buttonGap);

    setupButton(easyTexture, easyRect,
        mainLogoRect.y + mainLogoRect.h + buttonGap);
    easyRect.x = difficultyRect.x + difficultyRect.w + difficultyOptionGap;
    easyRect.y += difficultyRect.h / 4;
    easyRect.w /= 2;
    easyRect.h /= 2.5;

    setupButton(mediumTexture, mediumRect,   
        mainLogoRect.y + mainLogoRect.h + buttonGap);
    mediumRect.x = difficultyRect.x + difficultyRect.w + difficultyOptionGap;
    mediumRect.y = easyRect.y;
    mediumRect.w /= 2;
    mediumRect.h /= 2;

    setupButton(hardTexture, hardRect,   
        mainLogoRect.y + mainLogoRect.h + buttonGap);
    hardRect.x = difficultyRect.x + difficultyRect.w + difficultyOptionGap;
    hardRect.y = easyRect.y;
    hardRect.w /= 2;
    hardRect.h /= 2.5;

    setupButton(backTexture, backRect,   
        difficultyRect.y + difficultyRect.h + buttonGap);

    setupButton(quitTexture, quitRect,
        optionsRect.y + optionsRect.h + buttonGap);

    menuRects[MENU_START] = startRect;
    menuRects[MENU_OPTIONS] = optionsRect;
    menuRects[MENU_QUIT] = quitRect;

    optionRects[DIFFICULTY_SELECT] = mediumRect;
    optionRects[BACK] = backRect;

    // Cursor
    SDL_QueryTexture(cursorTexture, nullptr, nullptr, &texW, &texH);
    float cursorAspect = static_cast<float>(texH) / texW;

    cursorRect.w = static_cast<int>(screenW * 0.05f);
    cursorRect.h = static_cast<int>(cursorRect.w * cursorAspect);

    cursorX = menuRects[selectedIndex].x + menuRects[selectedIndex].w + 12.0f;
    cursorY = menuRects[selectedIndex].y;
    cursorRect.x = static_cast<int>(cursorX);
    cursorRect.y = static_cast<int>(cursorY);

    return true;
}

void MainMenu::handleInput(const SDL_Event& e, GameState& gs,
                           bool& running, bool& mRunning, Difficulty& difficulty)
{
    if (e.type != SDL_KEYDOWN) return;

    if (optionsOpen) {
        switch (e.key.keysym.sym)
        {
            case SDLK_UP:
                selectedIndexOptions = (selectedIndexOptions - 1 + OPTIONS_COUNT) % OPTIONS_COUNT;
                break;
    
            case SDLK_DOWN:
                selectedIndexOptions = (selectedIndexOptions + 1) % OPTIONS_COUNT;
                break;
    
            case SDLK_RETURN:
                activateSelected(gs, running, mRunning, difficulty);
                break;
    
            default:
                break;
        }
    }
    else {
        switch (e.key.keysym.sym)
        {
            case SDLK_UP:
                selectedIndex = (selectedIndex - 1 + MENU_COUNT) % MENU_COUNT;
                break;

            case SDLK_DOWN:
                selectedIndex = (selectedIndex + 1) % MENU_COUNT;
                break;

            case SDLK_RETURN:
                activateSelected(gs, running, mRunning, difficulty);
                break;

            default:
                break;
        }
    }
}

void MainMenu::updateCursor(float dt)
{
    if (optionsOpen) {
        const SDL_Rect& target = optionRects[selectedIndexOptions];
    
        float targetX = target.x + target.w + 12.0f;
        float targetY = target.y + target.h * 0.5f - cursorRect.h * 0.5f;
    
        if (selectedIndexOptions != lastSelectedIndex)
        {
            float dir = (selectedIndexOptions > lastSelectedIndex) ? 1.0f : -1.0f;
            cursorVY += dir * 180.0f;
            cursorScaleVel += 3.5f;
            lastSelectedIndex = selectedIndexOptions;
        }

        const float stiffness = 70.0f;
        const float damping   = 12.0f;

        cursorVX += (targetX - cursorX) * stiffness * dt;
        cursorVY += (targetY - cursorY) * stiffness * dt;

        cursorVX *= std::exp(-damping * dt);
        cursorVY *= std::exp(-damping * dt);

        cursorX += cursorVX * dt;
        cursorY += cursorVY * dt;

        const float scaleStiffness = 22.0f;
        const float scaleDamping   = 14.0f;

        cursorScaleVel += (1.0f - cursorScale) * scaleStiffness * dt;
        cursorScaleVel *= std::exp(-scaleDamping * dt);
        cursorScale += cursorScaleVel * dt;

        cursorRect.x = static_cast<int>(cursorX);
        cursorRect.y = static_cast<int>(cursorY);
    }
    else {
        const SDL_Rect& target = menuRects[selectedIndex];

        float targetX = target.x + target.w + 12.0f;
        float targetY = target.y + target.h * 0.5f - cursorRect.h * 0.5f;

        if (selectedIndex != lastSelectedIndex)
        {
            float dir = (selectedIndex > lastSelectedIndex) ? 1.0f : -1.0f;
            cursorVY += dir * 180.0f;
            cursorScaleVel += 3.5f;
            lastSelectedIndex = selectedIndex;
        }

        const float stiffness = 70.0f;
        const float damping   = 12.0f;

        cursorVX += (targetX - cursorX) * stiffness * dt;
        cursorVY += (targetY - cursorY) * stiffness * dt;

        cursorVX *= std::exp(-damping * dt);
        cursorVY *= std::exp(-damping * dt);

        cursorX += cursorVX * dt;
        cursorY += cursorVY * dt;

        const float scaleStiffness = 22.0f;
        const float scaleDamping   = 14.0f;

        cursorScaleVel += (1.0f - cursorScale) * scaleStiffness * dt;
        cursorScaleVel *= std::exp(-scaleDamping * dt);
        cursorScale += cursorScaleVel * dt;

        cursorRect.x = static_cast<int>(cursorX);
        cursorRect.y = static_cast<int>(cursorY);
    }
}

void MainMenu::activateSelected(GameState& gs,
                                bool& running, bool& mRunning, Difficulty& difficulty)
{
    if (optionsOpen) {
        switch (selectedIndexOptions)
        {
            case DIFFICULTY_SELECT:
                // Cycle through difficulties
                switch (difficulty)
                {
                    case Difficulty::Easy:
                        difficulty = Difficulty::Medium;
                        break;
                    case Difficulty::Medium:
                        difficulty = Difficulty::Hard;
                        break;
                    case Difficulty::Hard:
                        difficulty = Difficulty::Easy;
                        break;
                }
                break;
        
            case BACK:
                optionsOpen = false;
                break;        
        }
    }
    else {
        switch (selectedIndex)
        {
            case MENU_START:
                gs = GameState::Playing;
                break;

            case MENU_OPTIONS:
                optionsOpen = true;
                break;

            case MENU_QUIT:
                running = false;
                mRunning = false;
                break;
        }
    }
}

void MainMenu::spawnAshParticle()
{
    AshParticle p;

    p.x = mainLogoRect.x + rand() % mainLogoRect.w;
    p.y = mainLogoRect.y + rand() % (mainLogoRect.h / 2);

    p.size = 2.5f + (rand() % 100) / 100.0f * 2.5f;
    p.vx = ((rand() % 100) / 100.0f - 0.5f) * 4.0f;
    p.vy = -2.0f - (rand() % 2);
    p.life = 0.0f;
    p.maxLife = 6.5f + (rand() % 100) / 100.0f;

    ashParticles.push_back(p);
}

void MainMenu::updateAndRenderAsh(SDL_Renderer* renderer, float dt)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (size_t i = 0; i < ashParticles.size(); )
    {
        AshParticle& p = ashParticles[i];

        p.life += dt;
        if (p.life >= p.maxLife)
        {
            ashParticles.erase(ashParticles.begin() + i);
            continue;
        }

        p.x += p.vx * dt;
        p.y += p.vy * dt;

        float alpha = 1.0f - (p.life / p.maxLife);
        Uint8 a = static_cast<Uint8>(alpha * 180);

        float lifeT = p.life / p.maxLife;
        float size = p.size * (1.0f - lifeT * 0.3f);

        SDL_SetRenderDrawColor(renderer, 255, 183, 11, a);
        SDL_Rect r = {
            static_cast<int>(p.x - size * 0.5f),
            static_cast<int>(p.y - size * 0.5f),
            static_cast<int>(size),
            static_cast<int>(size)
        };
        SDL_RenderFillRect(renderer, &r);

        ++i;
    }
}

void MainMenu::render(SDL_Renderer* renderer, Difficulty& difficulty)
{
    SDL_RenderClear(renderer);

    // Main bg png
    SDL_RenderCopy(renderer, mainBgTexture, nullptr, &mainBgRect);

    // Button bg panel parameters
    const int panelWidth = screenW / 2 + cursorRect.w;
    const int fadeWidth = 80;

    // Main button bg panel
    SDL_Rect solidRect = { 0, 0, panelWidth - fadeWidth, screenH };
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 120, 25, 20, 80);
    SDL_RenderFillRect(renderer, &solidRect);

    // Feathered fade region
    for (int i = 0; i < fadeWidth; ++i)
    {
        float t = 1.0f - (float)i / fadeWidth;
        Uint8 alpha = static_cast<Uint8>(80 * t * t);

        SDL_SetRenderDrawColor(renderer, 120, 25, 20, alpha);

        SDL_Rect r = {
            solidRect.w + i,
            0,
            1,
            screenH
        };

        SDL_RenderFillRect(renderer, &r);
    }

    // Glow pulse
    float t = SDL_GetTicks() * 0.001f;
    float pulse = 0.75f + 0.25f * std::sin(t * 2.5f);
    pulse = std::clamp(pulse, 0.6f, 1.2f);
    Uint8 glow = static_cast<Uint8>(255 * pulse);

    SDL_SetTextureBlendMode(mainLogoBgTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(mainLogoBgTexture, glow, glow, glow);
    SDL_SetTextureAlphaMod(mainLogoBgTexture, 245);

    SDL_RenderCopy(renderer, mainLogoBgTexture, nullptr, &mainLogoRect);
    SDL_RenderCopy(renderer, mainLogoFgTexture, nullptr, &mainLogoRect);

    // Ash
    if (rand() % 100 < 3)
        spawnAshParticle();

    updateAndRenderAsh(renderer, 1.0f / 60.0f);

    // Buttons
    if (optionsOpen) {
        SDL_RenderCopy(renderer, difficultyTexture, nullptr, &difficultyRect);
        if (difficulty == Difficulty::Easy) {
            SDL_RenderCopy(renderer, easyTexture, nullptr, &easyRect);
        }
        else if (difficulty == Difficulty::Hard) {
            SDL_RenderCopy(renderer, hardTexture, nullptr, &hardRect);
        }
        else {
            SDL_RenderCopy(renderer, mediumTexture, nullptr, &mediumRect);
        }
        SDL_RenderCopy(renderer, backTexture, nullptr, &backRect);
    }
    else {
        SDL_RenderCopy(renderer, startTexture, nullptr, &startRect);
        SDL_RenderCopy(renderer, optionsTexture, nullptr, &optionsRect);
        SDL_RenderCopy(renderer, quitTexture, nullptr, &quitRect);
    }

    // Cursor render
    SDL_Rect renderRect = cursorRect;
    renderRect.w = static_cast<int>(cursorRect.w * cursorScale);
    renderRect.h = static_cast<int>(cursorRect.h * cursorScale);
    renderRect.x -= (renderRect.w - cursorRect.w) / 2;
    renderRect.y -= (renderRect.h - cursorRect.h) / 2;

    SDL_RenderCopy(renderer, cursorTexture, nullptr, &renderRect);
}

void MainMenu::shutdown()
{
    SDL_DestroyTexture(mainBgTexture);
    SDL_DestroyTexture(mainLogoFgTexture);
    SDL_DestroyTexture(mainLogoBgTexture);
    SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(optionsTexture);
    SDL_DestroyTexture(difficultyTexture);
    SDL_DestroyTexture(easyTexture);
    SDL_DestroyTexture(mediumTexture);
    SDL_DestroyTexture(hardTexture);
    SDL_DestroyTexture(backTexture);
    SDL_DestroyTexture(quitTexture);
    SDL_DestroyTexture(cursorTexture);
}
