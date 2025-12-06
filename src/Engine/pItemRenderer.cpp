#include "pItemRenderer.h"

void pItemRenderer::renderPItem(SDL_Renderer* renderer,
                                  SDL_Texture* itemTex,
                                  int screenWidth,
                                  int screenHeight,
                                  ItemType itemType,
                                  const WeaponManager& wm)
{
    // Normal weapon offset
    int xOffset = 0;
    int yOffset = 0;

    // "bobbing" offset
    float bobOffsetX = std::sin(wm.bobTimer) * wm.bobAmount;        // horizontal sway
    float bobOffsetY = std::fabs(std::cos(wm.bobTimer)) * (wm.bobAmount * 0.4f); // vertical bob

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
    dst.x = (screenWidth / 2) - (w / 2) + xOffset + bobOffsetX;
    dst.y = screenHeight - h + yOffset + bobOffsetY;

    SDL_RenderCopy(renderer, itemTex, NULL, &dst);

    // draw crosshair if the item is a gun
    if (isGun) {

        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        //  CENTER DOT
        int dotSize = 4;     
        SDL_Rect dot = {centerX - dotSize/2, centerY - dotSize/2, dotSize, dotSize};
        SDL_RenderFillRect(renderer, &dot);

        //  PISTOL RETICLE
        if (itemType == ItemType::Pistol) {
            int gap = 6;
            int length = 10;
            int thickness = 2;

            SDL_Rect left  = {centerX - gap - length, centerY - thickness/2, length, thickness};
            SDL_Rect right = {centerX + gap,         centerY - thickness/2, length, thickness};

            SDL_Rect up    = {centerX - thickness/2, centerY - gap - length, thickness, length};
            SDL_Rect down  = {centerX - thickness/2, centerY + gap,          thickness, length};

            SDL_RenderFillRect(renderer, &left);
            SDL_RenderFillRect(renderer, &right);
            SDL_RenderFillRect(renderer, &up);
            SDL_RenderFillRect(renderer, &down);
        }

        //  SHOTGUN RETICLE
        if (itemType == ItemType::Shotgun) {

            int radius = 20;        // circle size
            int thickness = 2;      // circle stroke thickness

            int cx = centerX;
            int cy = centerY;

            // Draw circle outline using midpoint circle algorithm
            for (int w = -thickness; w <= thickness; w++) {
                int r = radius + w;

                int x = r;
                int y = 0;
                int err = 1 - x;

                while (x >= y) {
                    SDL_RenderDrawPoint(renderer, cx + x, cy + y);
                    SDL_RenderDrawPoint(renderer, cx + y, cy + x);
                    SDL_RenderDrawPoint(renderer, cx - y, cy + x);
                    SDL_RenderDrawPoint(renderer, cx - x, cy + y);
                    SDL_RenderDrawPoint(renderer, cx - x, cy - y);
                    SDL_RenderDrawPoint(renderer, cx - y, cy - x);
                    SDL_RenderDrawPoint(renderer, cx + y, cy - x);
                    SDL_RenderDrawPoint(renderer, cx + x, cy - y);

                    y++;
                    if (err < 0) {
                        err += 2 * y + 1;
                    } else {
                        x--;
                        err += 2 * (y - x + 1);
                    }
                }
            }
        }

    }

}

