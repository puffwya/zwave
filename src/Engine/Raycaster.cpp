#include "Raycaster.h"
#include <cmath>
#include <vector>
#include <algorithm>

// ------------------------------------
// OLD RAYCASTER ENGINE NO LONGER IN USE
// ------------------------------------

// Draw a vertical line on the screen
inline void drawVerticalLine(uint32_t* pixels, int screenW, int screenH, int x, int startY, int endY, uint32_t 
color) {
    if (startY < 0) startY = 0;
    if (endY >= screenH) endY = screenH - 1;
    for (int y = startY; y <= endY; y++)
        pixels[y * screenW + x] = color;
}

// Draw a horizontal line on the screen
inline void drawHorizontalLine(uint32_t* pixels, int screenW, int screenH, int y, int startX, int endX, uint32_t 
color) {
    if (y < 0 || y >= screenH) return;
    if (startX < 0) startX = 0;
    if (endX >= screenW) endX = screenW - 1;
    for (int x = startX; x <= endX; x++)
        pixels[y * screenW + x] = color;
}

struct WallHit {
    float dist;   // distance from player
    float height; // partial height (0 < height <= 1)
    int side;     // 0=x-side, 1=y-side
    int mapX, mapY; // tile coordinates
};

void Raycaster::render(uint32_t* pixels, int screenW, int screenH,
                       const Player& player, const Map& map, float* zBuffer)
{
    constexpr uint32_t CEIL_COLOR  = 0xFF202040;
    constexpr uint32_t FLOOR_COLOR = 0xFF404020;
    constexpr uint32_t TOP_COLOR   = 0xFF808080; // flat top of partial walls

    for (int x = 0; x < screenW; x++)
    {
        // 1. Fill entire column with ceiling/floor first
        for (int y = 0; y < screenH/2; y++) pixels[y * screenW + x] = CEIL_COLOR;
        for (int y = screenH/2; y < screenH; y++) pixels[y * screenW + x] = FLOOR_COLOR;

        // 2. Ray setup
        float cameraX = 2.0f * x / float(screenW) - 1.0f;
        float rayDirX = cos(player.angle) + cameraX * -sin(player.angle);
        float rayDirY = sin(player.angle) + cameraX *  cos(player.angle);

        int mapX = int(player.x);
        int mapY = int(player.y);

        float deltaDistX = fabs(1 / rayDirX);
        float deltaDistY = fabs(1 / rayDirY);

        float sideDistX = (rayDirX < 0) ? (player.x - mapX) * deltaDistX
                                        : (mapX + 1.0f - player.x) * deltaDistX;
        float sideDistY = (rayDirY < 0) ? (player.y - mapY) * deltaDistY
                                        : (mapY + 1.0f - player.y) * deltaDistY;

        int stepX = (rayDirX < 0) ? -1 : 1;
        int stepY = (rayDirY < 0) ? -1 : 1;

        std::vector<WallHit> hits;

        int curX = mapX;
        int curY = mapY;
        float curSideDistX = sideDistX;
        float curSideDistY = sideDistY;

        // 3. DDA loop: collect all walls along this ray
        while (true)
        {
            int side = (curSideDistX < curSideDistY) ? 0 : 1;

            if (side == 0) { curSideDistX += deltaDistX; curX += stepX; }
            else           { curSideDistY += deltaDistY; curY += stepY; }

            if (curX < 0 || curX >= Map::SIZE || curY < 0 || curY >= Map::SIZE) break;

            const Map::Cell& cell = map.get(curX, curY);

            if (cell.type == Map::TileType::Wall && cell.height > 0.0f)
            {
                float dist = (side == 0) ? curSideDistX - deltaDistX : curSideDistY - deltaDistY;
                hits.push_back({ dist, cell.height, side, curX, curY });

                if (cell.height >= 1.0f) break; // full wall stops the ray
            }
        }

        if (hits.empty()) {
            zBuffer[x] = 1e6f;
            continue;
        }

        // 4. Sort hits back → front
        std::sort(hits.begin(), hits.end(), [](const WallHit& a, const WallHit& b){ return a.dist > b.dist; });

        float closestDist = 1e6f;

        // 5. Draw each wall slice
        for (const WallHit& hit : hits)
        {
            if (hit.dist < closestDist) closestDist = hit.dist;

            int fullLineHeight = int(screenH / hit.dist);
            int drawEnd   = (screenH / 2) + (fullLineHeight / 2);
            int drawStart = drawEnd - int(fullLineHeight * hit.height);

            if (drawStart < 0) drawStart = 0;
            if (drawEnd >= screenH) drawEnd = screenH - 1;

            uint32_t wallColor = (hit.side == 0) ? 0xFF00AAFF : 0xFF0055FF;
            drawVerticalLine(pixels, screenW, screenH, x, drawStart, drawEnd, wallColor);

            // Draw top surface for partial walls
            if (hit.height < 1.0f)
            {
                int topY = drawStart; // the top row of this partial wall

                // Determine horizontal extent of this tile’s top
                // Simple approach: extend ±1 pixel until next column hits a different tile
                int leftX = x, rightX = x;
                // Scan left
                while (leftX > 0)
                {
                    float cameraXl = 2.0f * leftX / float(screenW) - 1.0f;
                    float rayDirXL = cos(player.angle) + cameraXl * -sin(player.angle);
                    float rayDirYL = sin(player.angle) + cameraXl *  cos(player.angle);
                    int tileX = int(player.x + rayDirXL * hit.dist);
                    int tileY = int(player.y + rayDirYL * hit.dist);
                    if (tileX != hit.mapX || tileY != hit.mapY) break;
                    --leftX;
                }
                // Scan right
                while (rightX < screenW-1)
                {
                    float cameraXr = 2.0f * rightX / float(screenW) - 1.0f;
                    float rayDirXR = cos(player.angle) + cameraXr * -sin(player.angle);
                    float rayDirYR = sin(player.angle) + cameraXr *  cos(player.angle);
                    int tileX = int(player.x + rayDirXR * hit.dist);
                    int tileY = int(player.y + rayDirYR * hit.dist);
                    if (tileX != hit.mapX || tileY != hit.mapY) break;
                    ++rightX;
                }

                drawHorizontalLine(pixels, screenW, screenH, topY, leftX, rightX, wallColor);
            }
        }

        zBuffer[x] = closestDist;
    }
}

