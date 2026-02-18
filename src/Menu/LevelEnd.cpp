#include <SDL2/SDL_image.h>
#include <iostream>
#include "LevelEnd.h"

bool LevelEnd::init(SDL_Renderer* renderer, int screenW, int screenH) {
    width = screenW;
    height = screenH;

    SDL_Surface* surface = nullptr;

    // Load digits 0-9 + "/"
    for (int i = 0; i < 11; ++i) {
        std::string path = "Assets/pixDigit/pixelDigit-" + std::to_string(i) + ".png";

        SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
        if (!tex) {
            std::cerr << "Failed to load digit " << i
                      << " from path: " << path
                      << " | SDL_image error: " << IMG_GetError() << "\n";
            return false;
        }
        digitTextures[i] = tex;
    }

    // SECTOR png
    surface = IMG_Load("Assets/pixDigit/lvlEndSector.png");
    if (!surface) {
        std::cerr << "Failed to load sector png: " << IMG_GetError() << std::endl;
        return false;
    }

    sectorTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float sectorAspect = (float)surface->h / surface->w;

    sectorRect.w = (int)(screenW * 0.45f);
    sectorRect.h = (int)(sectorRect.w * sectorAspect);

    sectorRect.y = (screenH / 2) - sectorRect.h;

    // Start off-screen left
    sectorRect.x = -sectorRect.w;

    SDL_FreeSurface(surface);

    if (!sectorTexture) {
        std::cerr << "Failed to create sector texture\n";
        return false;
    }

    // PURGED png
    surface = IMG_Load("Assets/pixDigit/lvlEndPurged.png");
    if (!surface) {
        std::cerr << "Failed to load purged png: " << IMG_GetError() << std::endl;
        return false;
    }

    purgedTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float purgedAspect = (float)surface->h / surface->w;

    purgedRect.w = sectorRect.w;
    purgedRect.h = (int)(purgedRect.w * purgedAspect);

    purgedRect.y = (screenH / 2) + 10;

    // Start off-screen right
    purgedRect.x = screenW;

    SDL_FreeSurface(surface);

    if (!purgedTexture) {
        std::cerr << "Failed to create purged texture\n";
        return false;
    }

    // Target center positions
    sectorTargetX = (screenW / 2) - sectorRect.w;
    purgedTargetX = (screenW / 2);

    sectorYFloat = (float)sectorRect.y;
    purgedYFloat = (float)purgedRect.y;

    // Target y pos after slide up
    finalTopY = height / 6;

    int statBlockWidth = (int)(screenW * 0.25f);

    int statBlockX = (screenW - statBlockWidth) / 4;

    int statStartY = finalTopY + sectorRect.h + 150;

    surface = IMG_Load("Assets/pixDigit/enemiesTerm.png");
    if (!surface) {
        std::cerr << "Failed to load enemiesTerm png: " << IMG_GetError() << std::endl;
        return false;
    }

    enemiesTermTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float enemiesAspect = (float)surface->h / surface->w;

    enemiesTermRect.w = statBlockWidth;
    enemiesTermRect.h = (int)(statBlockWidth * enemiesAspect);

    // Center to the left of screen
    enemiesTermRect.x = statBlockX;

    // Vertical position (we’ll refine later during layout phase)
    enemiesTermRect.y = statStartY;

    SDL_FreeSurface(surface);

    if (!enemiesTermTexture) {
        std::cerr << "Failed to create enemiesTerm texture\n";
        return false;
    }

    surface = IMG_Load("Assets/pixDigit/shotsFired.png");
    if (!surface) {
        std::cerr << "Failed to load shotsFired png: " << IMG_GetError() << std::endl;
        return false;
    }

    shotsFiredTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float shotsAspect = (float)surface->h / surface->w;

    shotsFiredRect.w = statBlockWidth;
    shotsFiredRect.h = (int)(statBlockWidth * shotsAspect);

    shotsFiredRect.x = statBlockX;
    shotsFiredRect.y = enemiesTermRect.y + enemiesTermRect.h + 40;

    SDL_FreeSurface(surface);

    if (!shotsFiredTexture) {
        std::cerr << "Failed to create shotsFired texture\n";
        return false;
    }

    surface = IMG_Load("Assets/pixDigit/directHits.png");
    if (!surface) {
        std::cerr << "Failed to load directHits png: " << IMG_GetError() << std::endl;
        return false;
    }

    directHitsTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float hitsAspect = (float)surface->h / surface->w;

    directHitsRect.w = statBlockWidth;
    directHitsRect.h = (int)(statBlockWidth * hitsAspect);

    directHitsRect.x = statBlockX;
    directHitsRect.y = shotsFiredRect.y + shotsFiredRect.h + 40;

    SDL_FreeSurface(surface);

    if (!directHitsTexture) {
        std::cerr << "Failed to create directHits texture\n";
        return false;
    }

    surface = IMG_Load("Assets/pixDigit/accuracy.png");
    if (!surface) {
        std::cerr << "Failed to load accuracy png: " << IMG_GetError() << std::endl;
        return false;
    }

    accuracyTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float accuracyAspect = (float)surface->h / surface->w;

    accuracyRect.w = statBlockWidth;
    accuracyRect.h = (int)(statBlockWidth * accuracyAspect);

    accuracyRect.x = statBlockX;
    accuracyRect.y = directHitsRect.y + directHitsRect.h + 40;

    SDL_FreeSurface(surface);

    if (!accuracyTexture) {
        std::cerr << "Failed to create accuracy texture\n";
        return false;
    }

    surface = IMG_Load("Assets/pixDigit/timeElapsed.png");
    if (!surface) {
        std::cerr << "Failed to load timeElapsed png: " << IMG_GetError() << std::endl;
        return false;
    }

    timeElapsedTexture = SDL_CreateTextureFromSurface(renderer, surface);

    float timeAspect = (float)surface->h / surface->w;

    timeElapsedRect.w = statBlockWidth;
    timeElapsedRect.h = (int)(statBlockWidth * timeAspect);

    timeElapsedRect.x = statBlockX;
    timeElapsedRect.y = accuracyRect.y + accuracyRect.h + 40;

    SDL_FreeSurface(surface);

    if (!timeElapsedTexture) {
        std::cerr << "Failed to create timeElapsed texture\n";
        return false;
    }

    // For fade in
    SDL_SetTextureBlendMode(enemiesTermTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(shotsFiredTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(directHitsTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(accuracyTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(timeElapsedTexture, SDL_BLENDMODE_BLEND);

    return true;
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

void LevelEnd::renderStat(SDL_Renderer* renderer,
                          SDL_Texture* texture,
                          SDL_Rect rect,
                          float timer,
                          float appearTime,
                          int value)
{               
    if (timer < appearTime)
        return;
    
    float t = (timer - appearTime) / fadeDuration;
    if (t > 1.0f) t = 1.0f;
        
    Uint8 alpha = (Uint8)(255 * t);
        
    SDL_SetTextureAlphaMod(texture, alpha);        
    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    // Render digits underneath
    renderStatNumbers(renderer, rect, value, digitTextures, alpha);
}

void LevelEnd::renderStatNumbers(SDL_Renderer* renderer,
                                 SDL_Rect labelRect,
                                 int value,
                                 SDL_Texture* digitTextures[10],
                                 float alpha,
                                 int spacing)
{
    std::string valStr = std::to_string(value);

    // Digit size (match label height)
    int digitSize = labelRect.h;

    int x = labelRect.x + labelRect.w + 150;
    int y = labelRect.y;

    for (char c : valStr)
    {
        int d = c - '0';
        SDL_Rect dstDigit { x, y, digitSize, digitSize };

        // Apply same alpha as label for fade-in
        SDL_SetTextureAlphaMod(digitTextures[d], (Uint8)alpha);

        SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dstDigit);

        x += digitSize + spacing;
    }
}

void LevelEnd::update(float dt, GameState& gameState, AudioManager& audio)
{
    elapsedTime += dt;

    // Auto Return Timer
    if (elapsedTime >= returnDelay) {
        gameState = GameState::MainMenu;
        elapsedTime = 0.0f;
        return;
    }

    // Slide Start
    if (elapsedTime >= slideDelay && !collided)
    {
        if (!sliding) {
            sliding = true;
            audio.playSFX("lvlEnd_wordsCollide");
        }
    }

    // Sliding Logic
    if (sliding && !collided)
    {
        sectorRect.x += (int)(slideSpeed * dt);
        purgedRect.x -= (int)(slideSpeed * dt);

        if (sectorRect.x >= sectorTargetX)
        {
            sectorRect.x = sectorTargetX;
            purgedRect.x = purgedTargetX;

            collided = true;
            impactTime = impactDuration;
        }
    }

    // Impact Timer
    if (collided && impactTime > 0.0f)
    {
        impactTime -= dt;
    }

    // Trigger vertical slide after 4 seconds
    if (elapsedTime >= 4.0f && !slidingUp)
    {
        std::cout << "Slide Up Triggered\n";
        slidingUp = true;
    }

    // Vertical Slide Logic
    if (slidingUp)
    {
        if (sectorYFloat > finalTopY)
        {
            sectorYFloat -= slideUpSpeed * dt;
            purgedYFloat -= slideUpSpeed * dt;

            if (sectorYFloat <= finalTopY)
            {
                sectorYFloat = (float)finalTopY;
                purgedYFloat = (float)(finalTopY + sectorRect.h);
            }

            sectorRect.y = (int)sectorYFloat;
            purgedRect.y = (int)purgedYFloat;
        }

        if (sectorYFloat <= finalTopY)
        {
            sectorYFloat = (float)finalTopY;
            purgedYFloat = (float)(finalTopY + sectorRect.h);

            reportActive = true;
        }
    }

    if (reportActive)
    {
        reportTimer += dt;
    }

    // Ember Spawning
    if (embers.size() < maxEmbers)
    {
        Ember e;

        e.x = (float)(rand() % width);
        e.y = (float)(height + rand() % 30);

        e.speed = 20.0f + (rand() % 30);      // slower
        e.vx = -10.0f + (rand() % 20);        // gentle sideways drift

        e.size = 1.5f + (rand() % 2);         // smaller

        e.life = 4.0f + ((rand() % 100) / 100.0f);

        embers.push_back(e);
    }

    for (auto& e : embers)
    {
        e.y -= e.speed * dt;
        e.x += e.vx * dt;

        e.life -= dt;
    }

    // Remove dead embers
    embers.erase(
        std::remove_if(embers.begin(), embers.end(),
            [](Ember& e) { return e.life <= 0.0f; }),
        embers.end());
}

void LevelEnd::render(SDL_Renderer* renderer, Player& player, EnemyManager& em) {

    int shakeX = 0;
    int shakeY = 0;

    if (impactTime > 0.0f)
    {
        shakeX = rand() % 20 - 10;
        shakeY = rand() % 20 - 10;
    }

    // Black background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Bottom Glow
    int glowHeight = height / 2 + 200;

    for (int i = 0; i < glowHeight; i++)
    {
        float t = (float)i / glowHeight;

        // Much darker and more subtle
        Uint8 r = (Uint8)(120 * (1.0f - t));
        Uint8 g = (Uint8)(25 * (1.0f - t));
        Uint8 b = (Uint8)(10 * (1.0f - t));

        Uint8 alpha = (Uint8)(90 * (1.0f - t)); // lower alpha

        SDL_SetRenderDrawColor(renderer, r, g, b, alpha);

        SDL_Rect line = { 0, height - i, width, 1 };
        SDL_RenderFillRect(renderer, &line);
    }

    if (reportActive) 
    {
        // Grab stats from player / enemy manager
        int shotsFired = player.shotsFired;
        int shotsHit = player.shotsHit;
        int timeElapsed = player.timeElapsed;
        int enemiesKilled = em.enemiesKilled;
        int accuracy = (shotsFired > 0) ? (shotsHit * 100 / shotsFired) : 0;

        // Render each stat
        renderStat(renderer, enemiesTermTexture, enemiesTermRect, reportTimer, enemiesTime, enemiesKilled);
        renderStat(renderer, shotsFiredTexture, shotsFiredRect, reportTimer, shotsTime, shotsFired);
        renderStat(renderer, directHitsTexture, directHitsRect, reportTimer, hitsTime, shotsHit);
        renderStat(renderer, accuracyTexture, accuracyRect, reportTimer, accuracyTime, accuracy);
        renderStat(renderer, timeElapsedTexture, timeElapsedRect, reportTimer, timeTime, timeElapsed);
    }

    // Ember Rendering
    for (const auto& e : embers)
    {
        float lifeRatio = e.life / 3.0f;
        if (lifeRatio < 0) lifeRatio = 0;

        Uint8 alpha = (Uint8)(200 * lifeRatio);

        // darker ember tones
        Uint8 r = (Uint8)(200 * lifeRatio);
        Uint8 g = (Uint8)(80 * lifeRatio);
        Uint8 b = 20;

        SDL_SetRenderDrawColor(renderer, r, g, b, alpha);

        SDL_Rect emberRect =
        {
            (int)e.x,
            (int)e.y,
            (int)e.size,
            (int)e.size
        };

        SDL_RenderFillRect(renderer, &emberRect);
    }

    // Sector Purged rendering

    SDL_Rect sectorRender = sectorRect;
    SDL_Rect purgedRender = purgedRect;

    sectorRender.x += shakeX;
    sectorRender.y += shakeY;

    purgedRender.x += shakeX;
    purgedRender.y += shakeY;

    SDL_RenderCopy(renderer, sectorTexture, nullptr, &sectorRender);
    SDL_RenderCopy(renderer, purgedTexture, nullptr, &purgedRender);
}

void LevelEnd::resetAnimation()
{
    elapsedTime = 0.0f;

    sliding = false;
    slidingUp = false;
    collided = false;

    impactTime = 0.0f;

    // Reset positions
    sectorRect.x = -sectorRect.w;
    purgedRect.x = width;
    sectorRect.y = (height / 2) - sectorRect.h;
    purgedRect.y = (height / 2) + 10;

    sectorYFloat = (float)sectorRect.y;
    purgedYFloat = (float)purgedRect.y;

    reportActive = false;
    reportTimer = 0.0f;

    startedMusic = false;
}

void LevelEnd::cleanup() {
    if (sectorTexture) SDL_DestroyTexture(sectorTexture);
    if (purgedTexture) SDL_DestroyTexture(purgedTexture);
}

