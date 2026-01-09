#include "SpriteRenderer.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <iostream>

static std::unordered_map<SDL_Texture*, CachedTexture> textureCache;

static constexpr float FOV = 60.0f * (M_PI / 180.0f);

CachedTexture& SpriteRenderer::cacheTexture(SDL_Texture* tex) {
    auto it = textureCache.find(tex);
    if (it != textureCache.end())
        return it->second;

    CachedTexture ct;
    SDL_QueryTexture(tex, nullptr, nullptr, &ct.w, &ct.h);
    ct.pixels.resize(ct.w * ct.h);

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(
        0, ct.w, ct.h, 32, SDL_PIXELFORMAT_ARGB8888);

    SDL_Renderer* tmp = SDL_CreateSoftwareRenderer(surf);
    SDL_RenderCopy(tmp, tex, nullptr, nullptr);
    SDL_RenderPresent(tmp);

    std::memcpy(ct.pixels.data(), surf->pixels, ct.w * ct.h * 4);

    SDL_DestroyRenderer(tmp);
    SDL_FreeSurface(surf);

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
        if (!e.active || !e.sprite) continue;
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
        if (!e.active || !e.sprite) continue;

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
        int spriteW = spriteH; // keep square for now

        float spriteBottomZ = e->z;
        float spriteTopZ    = e->z + e->height;

        // If tile height is above sprite bottom, clip the sprite
        float visibleBottomZ = std::max(spriteBottomZ, tileHeight);

        // Vertical position on screen
        float enemyBottom = e->z - player.z;          // bottom relative to player
        float enemyTop    = e->z + e->height - player.z; // top relative to player

        int drawStartY = int(screenH / 2 - enemyTop / transformY * screenH);
        int drawEndY   = int(screenH / 2 - enemyBottom / transformY * screenH);

        int drawStartX = screenX - spriteW / 2;
        int drawEndX = drawStartX + spriteW;

        // Clip to screen
        drawStartX = std::max(0, drawStartX);
        drawEndX = std::min(screenW - 1, drawEndX);
        drawStartY = std::max(0, drawStartY);
        drawEndY = std::min(screenH - 1, drawEndY);

        // Render sprite as pixels (temporary placeholder: simple colored square)
        uint32_t color = 0xFFFF0000; // red placeholder

        // Draw sprites in proper world space using colWallTop and zBuffer
        for (int x = drawStartX; x < drawEndX; x++) {
            float wallDepth = zBuffer[x];

            // If wall is BEHIND sprite, draw normally
            if (wallDepth >= transformY) {
                for (int y = drawStartY; y < drawEndY; y++) {
                    pixels[y * screenW + x] = color;
                }
                continue;
            }

            // Wall is IN FRONT so may occlude
            int wallTopY = int(std::ceil(colWallTop[x]));

            // Fully blocked
            if (wallTopY <= drawStartY) {
                continue;
            }

            // Fully visible, don't occlude
            if (wallTopY >= drawEndY) {
                for (int y = drawStartY; y < drawEndY; y++) {
                    pixels[y * screenW + x] = color;
                }
                continue;
            }

            // Partially occluded
            for (int y = drawStartY; y < wallTopY; y++) {
                pixels[y * screenW + x] = color;
            }
        }
    }
}
