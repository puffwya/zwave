#include "PickupManager.h"
#include "Player.h"
#include "Map.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <SDL2/SDL_image.h>

// Helper to load a PNG into a PickupVisual
bool PickupManager::loadPickupFrame(const std::string& path, PickupVisual& out) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cerr << "Failed to load pickup: " << path << "\n";
        return false;
    }

    SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);
    if (!formatted) return false;

    out.w = formatted->w;
    out.h = formatted->h;
    out.pixels.resize(out.w * out.h);
    std::memcpy(out.pixels.data(), formatted->pixels, out.w * out.h * 4);
    SDL_FreeSurface(formatted);

    return true;
}

// Load all pickup textures
void PickupManager::loadPickupAssets() {
    PickupVisual health;
    if (!loadPickupFrame("Assets/Pickups/health.png", health))
        std::cerr << "Failed to load health pickup\n";
    pickupsVisuals[PickupType::Health] = health;

    PickupVisual armor;
    if (!loadPickupFrame("Assets/Pickups/armor.png", armor))
        std::cerr << "Failed to load armor pickup\n";
    pickupsVisuals[PickupType::Armor] = armor;

    PickupVisual ammo;
    if (!loadPickupFrame("Assets/Pickups/ammo.png", ammo))
        std::cerr << "Failed to load ammo pickup\n";
    pickupsVisuals[PickupType::Ammo] = ammo;

    PickupVisual weapon;
    if (!loadPickupFrame("Assets/Pickups/weapon.png", weapon))
        std::cerr << "Failed to load weapon pickup\n";
    pickupsVisuals[PickupType::Weapon] = weapon;
}

// Add a pickup to the world
void PickupManager::addPickup(float x, float y, float z, PickupType type) {
    if (pickups.size() >= MAX_PICKUPS) return;

    Pickup p;
    p.x = x;
    p.y = y;
    p.z = z;
    p.type = type;
    p.active = true;
    p.visual = &pickupsVisuals.at(type);

    pickups.push_back(p);
}

// Render pickups in world space
void PickupManager::renderPickups(
    uint32_t* pixels,
    int screenW,
    int screenH,
    const Player& player,
    float* zBuffer,
    Map& map,
    float colWallTop[]
) {
    struct DrawInfo {
        Pickup* pickup;
        float dist;
    };

    DrawInfo drawList[128]; // adjust max pickups if needed
    int count = 0;

    // Collect active pickups and compute distance to player
    for (Pickup& p : pickups) {
        if (!p.active) continue;
        float dx = p.x - player.x;
        float dy = p.y - player.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        drawList[count++] = { &p, dist };
    }

    if (count == 0) return;

    // Sort back-to-front
    std::sort(drawList, drawList + count,
        [](const DrawInfo& a, const DrawInfo& b) { return a.dist > b.dist; }
    );

    // Camera vectors
    float dirX = std::cos(player.angle);
    float dirY = std::sin(player.angle);
    float planeX = -dirY * 0.66f;
    float planeY = dirX * 0.66f;

    for (int i = 0; i < count; i++) {
        Pickup* p = drawList[i].pickup;
        PickupVisual& v = *p->visual;

        // Compute relative position in world plane (ignore z for horizontal projection)
        float dx = p->x - player.x;
        float dy = p->y - player.y;

        // Transform to camera space
        float invDet = 1.0f / (planeX * dirY - dirX * planeY);
        float transformX = invDet * ( dirY * dx - dirX * dy );
        float transformY = invDet * (-planeY * dx + planeX * dy );

        if (transformY <= 0.05f) continue; // behind camera

        // Screen X position
        int screenX = int((screenW / 2.0f) * (1 + transformX / transformY));

        // Vertical scaling
        float pickupHeight = 0.25f; // world units
        int spriteH = std::max(1, int(screenH / transformY * pickupHeight));
        int spriteW = std::max(1, int(spriteH * (float(v.w) / float(v.h)))); // preserve aspect ratio

        // Vertical position (z offset)
        float pickupBottom = p->z - player.z;
        float pickupTop    = p->z + pickupHeight - player.z;

        int drawStartY = int(screenH / 2 - pickupTop / transformY * screenH);
        int drawEndY   = int(screenH / 2 - pickupBottom / transformY * screenH);
        int drawStartX = screenX - spriteW / 2;
        int drawEndX   = drawStartX + spriteW;

        // Clamp to screen
        drawStartX = std::max(0, drawStartX);
        drawEndX   = std::min(screenW - 1, drawEndX);
        drawStartY = std::max(0, drawStartY);
        drawEndY   = std::min(screenH - 1, drawEndY);

        int spanW = drawEndX - drawStartX;
        int spanH = drawEndY - drawStartY;
        if (spanW <= 0 || spanH <= 0) continue;

        // Draw pixel lambda
        auto drawPixel = [&](int x, int y) {
            int srcX = std::clamp((x - drawStartX) * v.w / spanW, 0, v.w - 1);
            int srcY = std::clamp((y - drawStartY) * v.h / spanH, 0, v.h - 1);
            uint32_t color = v.pixels[srcY * v.w + srcX];
            if ((color >> 24) == 0) return; // skip transparent
            pixels[y * screenW + x] = color;
        };

        // Draw with z-buffer / wall clipping
        for (int x = drawStartX; x < drawEndX; x++) {
            float wallDepth = zBuffer[x];
            if (wallDepth >= transformY) {
                for (int y = drawStartY; y < drawEndY; y++)
                    drawPixel(x, y);
                continue;
            }

            int wallTopY = int(std::ceil(colWallTop[x]));
            if (wallTopY <= drawStartY) continue;
            int clipEnd = std::min(wallTopY, drawEndY);
            for (int y = drawStartY; y < clipEnd; y++)
                drawPixel(x, y);
        }
    }
}

