#include "pItemRenderer.h"

void pItemRenderer::renderPItem(SDL_Renderer* renderer,
                                  SDL_Texture* itemTex,
                                  int screenWidth,
                                  int screenHeight,
                                  ItemType itemType)
{
    int xOffset = 0;
    int yOffset = 0;
    bool isGun = false;

    // scale
    float scale = 2.0f; // default size of an item

    // determine offsets and gun status
    switch (itemType) {
        case ItemType::Pistol:
            xOffset = -100;
            yOffset = 5;
            scale = 2.25;
            isGun = true;
            break;
        case ItemType::Shotgun:
            isGun = true;
            break;
        default:
            isGun = false;
            break;
    }

    int w, h;
    SDL_QueryTexture(itemTex, NULL, NULL, &w, &h);

    w = int(w * scale);
    h = int(h * scale);

    SDL_Rect dst;
    dst.w = w;
    dst.h = h;
    dst.x = (screenWidth / 2) - (w / 2) + xOffset;
    dst.y = screenHeight - h + yOffset;

    SDL_RenderCopy(renderer, itemTex, NULL, &dst);

    // draw crosshair if the item is a gun
    if (isGun) {

        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;

        int gap = 6;      // space between center dot and start of lines
        int length = 10;  // length of each line
        int thickness = 2; // line thickness

        // set crosshair color
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white

        // horizontal lines
        SDL_Rect left  = {centerX - gap - length, centerY - thickness / 2, length, thickness};
        SDL_Rect right = {centerX + gap, centerY - thickness / 2, length, thickness};

        // vertical lines
        SDL_Rect up    = {centerX - thickness / 2, centerY - gap - length, thickness, length};
        SDL_Rect down  = {centerX - thickness / 2, centerY + gap, thickness, length};

        SDL_RenderFillRect(renderer, &left);
        SDL_RenderFillRect(renderer, &right);
        SDL_RenderFillRect(renderer, &up);
        SDL_RenderFillRect(renderer, &down);

        // center dot
        int dotSize = 4;
        SDL_Rect centerDot = {centerX - dotSize / 2, centerY - dotSize / 2, dotSize, dotSize};
        SDL_RenderFillRect(renderer, &centerDot);
    }
}

