#include <SDL2/SDL_image.h>
#include <iostream>
#include <algorithm>
#include "LevelEnd.h"

constexpr int STAT_VERTICAL_SPACING = 40;
constexpr int DIGIT_OFFSET_X = 150;

SDL_Texture* LevelEnd::loadTexture(SDL_Renderer* renderer, const std::string& path)
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

bool LevelEnd::init(SDL_Renderer* renderer, int screenW, int screenH)
{
    width = screenW;
    height = screenH;

    // Load digits 0-9
    for (int i = 0; i < DIGIT_COUNT; ++i)
    {
        std::string path = "Assets/pixDigit/pixelDigit-" + std::to_string(i) + ".png";
        digitTextures[i] = loadTexture(renderer, path);
        if (!digitTextures[i]) return false;
    }

    sectorTexture = loadTexture(renderer, "Assets/pixDigit/lvlEndSector.png");
    purgedTexture = loadTexture(renderer, "Assets/pixDigit/lvlEndPurged.png");

    if (!sectorTexture || !purgedTexture) return false;

    // Setup title layout
    int sectorW = static_cast<int>(screenW * 0.45f);

    int texW, texH;
    SDL_QueryTexture(sectorTexture, nullptr, nullptr, &texW, &texH);
    float sectorAspect = static_cast<float>(texH) / texW;

    sectorRect.w = sectorW;
    sectorRect.h = static_cast<int>(sectorW * sectorAspect);
    sectorRect.x = -sectorRect.w;
    sectorRect.y = (screenH / 2) - sectorRect.h;

    SDL_QueryTexture(purgedTexture, nullptr, nullptr, &texW, &texH);
    float purgedAspect = static_cast<float>(texH) / texW;

    purgedRect.w = sectorW;
    purgedRect.h = static_cast<int>(sectorW * purgedAspect);
    purgedRect.x = screenW;
    purgedRect.y = (screenH / 2) + 10;

    sectorTargetX = (screenW / 2) - sectorRect.w;
    purgedTargetX = (screenW / 2);

    sectorYFloat = static_cast<float>(sectorRect.y);
    purgedYFloat = static_cast<float>(purgedRect.y);

    finalTopY = height / 6;

    int statBlockWidth = static_cast<int>(screenW * 0.25f);
    int statBlockX = (screenW - statBlockWidth) / 4;
    int statStartY = finalTopY + sectorRect.h + 150;

    auto setupStat = [&](SDL_Texture*& texture,
                         SDL_Rect& rect,
                         const std::string& path,
                         int y)
    {
        texture = loadTexture(renderer, path);
        if (!texture) return false;

        int w, h;
        SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
        float aspect = static_cast<float>(h) / w;

        rect.w = statBlockWidth;
        rect.h = static_cast<int>(statBlockWidth * aspect);
        rect.x = statBlockX;
        rect.y = y;

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        return true;
    };

    if (!setupStat(enemiesTermTexture, enemiesTermRect,
                   "Assets/pixDigit/enemiesTerm.png",
                   statStartY)) return false;

    if (!setupStat(shotsFiredTexture, shotsFiredRect,
                   "Assets/pixDigit/shotsFired.png",
                   enemiesTermRect.y + enemiesTermRect.h + STAT_VERTICAL_SPACING)) return false;

    if (!setupStat(directHitsTexture, directHitsRect,
                   "Assets/pixDigit/directHits.png",
                   shotsFiredRect.y + shotsFiredRect.h + STAT_VERTICAL_SPACING)) return false;

    if (!setupStat(accuracyTexture, accuracyRect,
                   "Assets/pixDigit/accuracy.png",
                   directHitsRect.y + directHitsRect.h + STAT_VERTICAL_SPACING)) return false;

    if (!setupStat(timeElapsedTexture, timeElapsedRect,
                   "Assets/pixDigit/timeElapsed.png",
                   accuracyRect.y + accuracyRect.h + STAT_VERTICAL_SPACING)) return false;

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

    Uint8 alpha = static_cast<Uint8>(255 * t);

    SDL_SetTextureAlphaMod(texture, alpha);
    SDL_RenderCopy(renderer, texture, nullptr, &rect);

    renderStatNumbers(renderer, rect, value, alpha);
}

void LevelEnd::renderStatNumbers(SDL_Renderer* renderer,
                                 SDL_Rect labelRect,
                                 int value,
                                 float alpha,
                                 int spacing)
{
    std::string valStr = std::to_string(value);

    int digitSize = labelRect.h;
    int x = labelRect.x + labelRect.w + DIGIT_OFFSET_X;
    int y = labelRect.y;

    for (char c : valStr)
    {
        int d = c - '0';
        if (d < 0 || d >= DIGIT_COUNT)
            continue;

        SDL_Rect dst { x, y, digitSize, digitSize };

        SDL_SetTextureAlphaMod(digitTextures[d], static_cast<Uint8>(alpha));
        SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dst);

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

void LevelEnd::cleanup()
{
    for (int i = 0; i < DIGIT_COUNT; ++i)
        if (digitTextures[i])
            SDL_DestroyTexture(digitTextures[i]);

    SDL_DestroyTexture(sectorTexture);
    SDL_DestroyTexture(purgedTexture);
    SDL_DestroyTexture(enemiesTermTexture);
    SDL_DestroyTexture(shotsFiredTexture);
    SDL_DestroyTexture(directHitsTexture);
    SDL_DestroyTexture(accuracyTexture);
    SDL_DestroyTexture(timeElapsedTexture);
}
