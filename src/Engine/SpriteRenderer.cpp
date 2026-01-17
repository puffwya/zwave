#include "SpriteRenderer.h"
#include <cmath>
#include <algorithm>
#include <iostream>

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
    // Collect active enemies
    struct DrawInfo {
        Enemy* enemy;
        float dist;
    };

    DrawInfo drawList[EnemyManager::MAX_ENEMIES];
    int count = 0;

    for (Enemy& e : manager.enemies) {
        if (!e.active) continue;

        float dx = e.x - player.x;
        float dy = e.y - player.y;
        drawList[count++] = { &e, std::sqrt(dx*dx + dy*dy) };
    }

    if (count == 0) return;

    // Painter's algorithm
    std::sort(drawList, drawList + count,
        [](const DrawInfo& a, const DrawInfo& b) {
            return a.dist > b.dist;
        }
    );

    // Camera basis
    float dirX = std::cos(player.angle);
    float dirY = std::sin(player.angle);
    float planeX = -dirY * 0.66f;
    float planeY = dirX * 0.66f;

    for (int i = 0; i < count; i++) {
        Enemy* e = drawList[i].enemy;

        const EnemyVisual& visual =
            manager.enemyVisuals.at(e->type);

        const Animation& anim =
            visual.animations.at(e->animState);

        int frameIndex = e->animFrame % anim.frames.size();
        const SpriteFrame& frame = anim.frames[frameIndex];

        float dx = e->x - player.x;
        float dy = e->y - player.y;

        float invDet = 1.0f / (planeX * dirY - dirX * planeY);
        float transformX = invDet * ( dirY * dx - dirX * dy );
        float transformY = invDet * (-planeY * dx + planeX * dy );

        if (transformY <= 0.05f) continue;

        int screenX = int((screenW / 2.0f) * (1 + transformX / transformY));
        int spriteH = std::max(1, int(screenH / transformY * e->height));
        int spriteW = spriteH;

        float enemyBottom = e->z - player.z;
        float enemyTop    = e->z + e->height - player.z;

        int drawStartY = int(screenH / 2 - enemyTop    / transformY * screenH);
        int drawEndY = int(screenH / 2 - enemyBottom / transformY * screenH);
        int drawStartX = screenX - spriteW / 2;
        int drawEndX = drawStartX + spriteW;

        drawStartX = std::max(0, drawStartX);
        drawEndX = std::min(screenW - 1, drawEndX);
        drawStartY = std::max(0, drawStartY);
        drawEndY = std::min(screenH - 1, drawEndY);

        int spanW = drawEndX - drawStartX;
        int spanH = drawEndY - drawStartY;

        if (spanW <= 0 || spanH <= 0) continue;

        // Helper: sample & draw pixel
        auto drawPixel = [&](int x, int y) {
            int srcX = (x - drawStartX) * frame.w / spanW;
            int srcY = (y - drawStartY) * frame.h / spanH;

            srcX = std::clamp(srcX, 0, frame.w - 1);
            srcY = std::clamp(srcY, 0, frame.h - 1);

            uint32_t color = frame.pixels[srcY * frame.w + srcX];
            if ((color >> 24) == 0) return;

            pixels[y * screenW + x] = color;
        };

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

