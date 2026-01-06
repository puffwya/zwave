#include "HUD.h"
#include <SDL2/SDL_image.h>
#include <iostream>

bool HUD::loadDigitTextures(SDL_Renderer* renderer) {
    for (int i = 0; i < 11; ++i) {
        // Construct path
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

    // Get width/height from first digit for layout purposes
    if (digitTextures[0]) {
        SDL_QueryTexture(digitTextures[0], nullptr, nullptr, &digitW, &digitH);
    }

    return true;
}

void HUD::drawAmmo(SDL_Renderer* renderer,
                   int clip,
                   int reserve,
                   int x,
                   int y,
                   int scale)
{
    // convert both numbers to strings
    std::string clipStr = std::to_string(clip);
    std::string reserveStr = std::to_string(reserve);

    int spacing = 2 * scale;

    // calculate total width: clip + separator + reserve
    int totalW = (clipStr.size() + 1 + reserveStr.size()) * (digitW * scale) 
                 + ((clipStr.size() + 1 + reserveStr.size() - 1) * spacing);

    // right align at x
    int drawX = x - totalW;

    // draw clip digits
    for (char c : clipStr) {
        int d = c - '0';
        SDL_Rect dst{ drawX, y, digitW * scale, digitH * scale };
        SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dst);
        drawX += digitW * scale + spacing;
    }

    // draw separator "/"
    SDL_Rect slashDst{ drawX, y, digitW * scale, digitH * scale };
    SDL_RenderCopy(renderer, digitTextures[10], nullptr, &slashDst); // digitTextures[10] = "/"
    drawX += digitW * scale + spacing;

    // draw reserve digits
    for (char c : reserveStr) {
        int d = c - '0';
        SDL_Rect dst{ drawX, y, digitW * scale, digitH * scale };
        SDL_RenderCopy(renderer, digitTextures[d], nullptr, &dst);
        drawX += digitW * scale + spacing;
    }
}

// Creates health and armor bars
void HUD::drawBar(SDL_Renderer* renderer,
                  int x, int y, int w, int h,
                  int value, int maxValue,
                  SDL_Color color)
{
    float pct = (float)value / (float)maxValue;
    pct = std::clamp(pct, 0.0f, 1.0f);

    SDL_Rect bg { x, y, w, h };
    SDL_Rect fg { x, y, (int)(w * pct), h };

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &bg);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &fg);
}

void HUD::render(SDL_Renderer* renderer,
                 const Player& player,
                 int screenW,
                 int screenH, Weapon& weapon)
{
    const int margin = 20;
    const int barW = 200;
    const int barH = 18;

    // Health (left)
    drawBar(renderer,
            margin,
            screenH - margin - barH,
            barW, barH,
            player.health, player.maxHealth,
            { 200, 40, 40 });

    // Armor (left above health)
    drawBar(renderer,
            margin,
            screenH - margin - barH * 2 - 6,
            barW, barH,
            player.armor, player.maxArmor,
            { 40, 40, 200 });

    float hudScaleF = (float)screenH / 720.0f;
    hudScaleF *= 0.75f;

    int hudScale = std::max((int)hudScaleF, 1);

    int ammoX = screenW - (screenW * 0.02f);
    int ammoY = screenH - (screenH * 0.05f);

    ammoX = (ammoX / hudScale) * hudScale;
    ammoY = (ammoY / hudScale) * hudScale;

    // ammo count
    if (player.currentItem == ItemType::Pistol) {
        drawAmmo(renderer,
             weapon.pClipAmmo,
             weapon.pReserveAmmo,
             ammoX,
             ammoY,
             hudScale);
    }
    else if (player.currentItem == ItemType::Shotgun) {
        drawAmmo(renderer,
             weapon.sClipAmmo,
             weapon.sReserveAmmo,
             ammoX,
             ammoY,
             hudScale);  
    }
}
