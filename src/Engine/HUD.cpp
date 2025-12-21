#include "HUD.h"
#include <SDL2/SDL_image.h>
#include <iostream>

bool HUD::loadDigitTextures(SDL_Renderer* renderer) {
    for (int i = 0; i < 10; ++i) {
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

void HUD::drawDigits(SDL_Renderer* renderer,
                     int value,
                     int x,
                     int y,
                     bool rightAlign)
{
    std::string str = std::to_string(value);

    int scale = 2;               // tweak later
    int spacing = 2 * scale;
    int totalW =
        str.size() * (digitW * scale + spacing) - spacing;

    int drawX = rightAlign ? (x - totalW) : x;

    for (char c : str) {
        int d = c - '0';

        SDL_Rect dst{
            drawX,
            y,
            digitW * scale,
            digitH * scale
        };

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
                 int screenH)
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

    // ammo count
    int ammo = player.ammo;

    drawDigits(renderer,
           ammo,
           screenW - 16,
           screenH - 48,
           true);
}

