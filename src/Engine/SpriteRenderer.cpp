#include "SpriteRenderer.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <iostream>

static std::unordered_map<SDL_Texture*, CachedTexture> textureCache;

static constexpr float FOV = 60.0f * (M_PI / 180.0f);

CachedTexture& SpriteRenderer::cacheTexture(SDL_Texture* tex) {
    // Check if already cached
    auto it = textureCache.find(tex);
    if (it != textureCache.end())
        return it->second;

    CachedTexture ct;

    // Get texture size
    int w, h;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    ct.w = w;
    ct.h = h;
    ct.pixels.resize(w * h);

    // Create a temporary surface to read pixels
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!surf) {
        std::cerr << "Failed to create surface for texture caching: " 
                  << SDL_GetError() << std::endl;
        return textureCache[tex] = std::move(ct);
    }

    // Create temporary renderer to copy texture to surface
    SDL_Renderer* tmpRenderer = SDL_CreateSoftwareRenderer(surf);
    if (!tmpRenderer) {
        std::cerr << "Failed to create temporary renderer: " 
                  << SDL_GetError() << std::endl;
        SDL_FreeSurface(surf);
        return textureCache[tex] = std::move(ct);
    }

    SDL_SetRenderTarget(tmpRenderer, nullptr);
    SDL_RenderCopy(tmpRenderer, tex, nullptr, nullptr);
    SDL_RenderPresent(tmpRenderer);

    // Copy pixels row by row safely
    for (int y = 0; y < h; y++) {
        uint32_t* dstRow = ct.pixels.data() + y * w;
        Uint8* srcRow = (Uint8*)surf->pixels + y * surf->pitch;
        for (int x = 0; x < w; x++) {
            dstRow[x] = ((uint32_t*)srcRow)[x];
        }
    }

    SDL_DestroyRenderer(tmpRenderer);
    SDL_FreeSurface(surf);

    // Store in cache and return
    return textureCache[tex] = std::move(ct);
}

void SpriteRenderer::renderEnemies(
    uint32_t* pixels,
    int screenW,
    int screenH,
    EnemyManager& manager,
    const Player& player,
    float* zBuffer,
    Map& map,
    float colWallTop[]
) {
    const float FOV = 66.0f * (3.14159265f / 180.0f); // convert to radians

    // Step 1: Count active enemies
    int activeCount = 0;
    for (int i = 0; i < EnemyManager::MAX_ENEMIES; i++) {
        Enemy& e = manager.enemies[i];
        if (!e.active || e.spritePixels.empty()) continue;
        activeCount++;
    }

    if (activeCount == 0) return; // nothing to render

    // Step 2: Build an array of active enemies and their distances
    struct DrawInfo {
        Enemy* enemy;
        float dist;
    };

    DrawInfo drawList[EnemyManager::MAX_ENEMIES];
    int count = 0;

    for (int i = 0; i < EnemyManager::MAX_ENEMIES; i++) {
        Enemy& e = manager.enemies[i];
        if (!e.active || e.spritePixels.empty()) continue;

        float dx = e.x - player.x;
        float dy = e.y - player.y;
        float dist = std::sqrt(dx*dx + dy*dy);

        drawList[count++] = { &e, dist };
    }

    // Step 3: Sort enemies farthest to nearest (painter's algorithm)
    std::sort(drawList, drawList + count, [](const DrawInfo& a, const DrawInfo& b) {
        return a.dist > b.dist;
    });

    // Step 4: Render each enemy
    for (int i = 0; i < count; i++) {
        Enemy* e = drawList[i].enemy;

        if (!e->active || e->spritePixels.empty()) continue;

        // Vector from player to enemy
        float dx = e->x - player.x;
        float dy = e->y - player.y;

        int tileX = int(e->x);
        int tileY = int(e->y);
        float tileHeight = map.get(tileX, tileY).height; // 0..1

        // Transform into camera space
        float dirX = std::cos(player.angle);
        float dirY = std::sin(player.angle);
        float planeX = -dirY * 0.66f; // camera plane
        float planeY = dirX * 0.66f;

        float invDet = 1.0f / (planeX * dirY - dirX * planeY);
        float transformX = invDet * ( dirY * dx - dirX * dy );
        float transformY = invDet * (-planeY * dx + planeX * dy );

        if (transformY <= 0.05f) continue; // behind camera

        // Projected screen X
        int screenX = int((screenW / 2.0f) * (1 + transformX / transformY));

        // Scale sprite based on distance
        int spriteH = std::max(1, int(screenH / transformY * e->height));
        int spriteW = spriteH; // square for now

        // Vertical position on screen
        float enemyBottom = e->z - player.z;
        float enemyTop    = e->z + e->height - player.z;

        int drawStartY = int(screenH / 2 - enemyTop / transformY * screenH);
        int drawEndY   = int(screenH / 2 - enemyBottom / transformY * screenH);

        int drawStartX = screenX - spriteW / 2;
        int drawEndX   = drawStartX + spriteW;

        // Clip to screen
        drawStartX = std::max(0, drawStartX);
        drawEndX   = std::min(screenW - 1, drawEndX);
        drawStartY = std::max(0, drawStartY);
        drawEndY   = std::min(screenH - 1, drawEndY);

        // Draw sprite in screen space with wall occlusion
        for (int x = drawStartX; x < drawEndX; x++) {
            float wallDepth = zBuffer[x];

            // If wall is behind sprite, draw full column
            if (wallDepth >= transformY) {
                for (int y = drawStartY; y < drawEndY; y++) {
                    int srcX = (x - drawStartX) * e->spriteW / (drawEndX - drawStartX);
                    int srcY = (y - drawStartY) * e->spriteH / (drawEndY - drawStartY);
                    uint32_t color = e->spritePixels[srcY * e->spriteW + srcX];

                    // Skip fully transparent pixels (ARGB)
                    if ((color >> 24) == 0) continue;

                    pixels[y * screenW + x] = color;
                }
                continue;
            }

            // Wall in front → possible partial occlusion
            int wallTopY = int(std::ceil(colWallTop[x]));

            // Fully blocked
            if (wallTopY <= drawStartY) continue;

            // Fully visible, wall below sprite
            if (wallTopY >= drawEndY) {
                for (int y = drawStartY; y < drawEndY; y++) {
                    int srcX = (x - drawStartX) * e->spriteW / (drawEndX - drawStartX);
                    int srcY = (y - drawStartY) * e->spriteH / (drawEndY - drawStartY);
                    uint32_t color = e->spritePixels[srcY * e->spriteW + srcX];
                    if ((color >> 24) == 0) continue;
                    pixels[y * screenW + x] = color;
                }
                continue;
            }

            // Partial occlusion (clip top)
            for (int y = drawStartY; y < wallTopY; y++) {
                int srcX = (x - drawStartX) * e->spriteW / (drawEndX - drawStartX);
                int srcY = (y - drawStartY) * e->spriteH / (drawEndY - drawStartY);
                uint32_t color = e->spritePixels[srcY * e->spriteW + srcX];
                if ((color >> 24) == 0) continue;
                pixels[y * screenW + x] = color;
            }
        }
    }

}
