#include "Raycaster.h"
#include <cmath>
#include <vector>
#include <algorithm>

// Draw a vertical line on the screen
inline void drawVerticalLine(uint32_t* pixels, int screenW, int screenH, int x, int startY, int endY, uint32_t 
color) {
    if (startY < 0) startY = 0;
    if (endY >= screenH) endY = screenH - 1;
    for (int y = startY; y <= endY; y++)
        pixels[y * screenW + x] = color;
}

struct WallHit {
    float dist;
    float height;
    int side;
};

void Raycaster::render(uint32_t* pixels, int screenW, int screenH,
                       const Player& player, const Map& map, float* zBuffer)
{
    for (int x = 0; x < screenW; x++)
    {
        float cameraX = 2.0f * x / float(screenW) - 1.0f;
        float rayDirX = cos(player.angle) + cameraX * -sin(player.angle);
        float rayDirY = sin(player.angle) + cameraX *  cos(player.angle);

        int mapX = int(player.x);
        int mapY = int(player.y);

        float deltaDistX = fabs(1 / rayDirX);
        float deltaDistY = fabs(1 / rayDirY);

        float sideDistX, sideDistY;
        int stepX, stepY;

        if (rayDirX < 0) { stepX = -1; sideDistX = (player.x - mapX) * deltaDistX; }
        else { stepX = 1; sideDistX = (mapX + 1.0 - player.x) * deltaDistX; }

        if (rayDirY < 0) { stepY = -1; sideDistY = (player.y - mapY) * deltaDistY; }
        else { stepY = 1; sideDistY = (mapY + 1.0 - player.y) * deltaDistY; }

        std::vector<WallHit> hits;

        int currentMapX = mapX;
        int currentMapY = mapY;
        float currentSideDistX = sideDistX;
        float currentSideDistY = sideDistY;

        // Cast the ray, storing all hits along the way
        while (true)
        {
            int hitSide = (currentSideDistX < currentSideDistY) ? 0 : 1;

            if (hitSide == 0) { currentSideDistX += deltaDistX; currentMapX += stepX; }
            else { currentSideDistY += deltaDistY; currentMapY += stepY; }

            if (currentMapX < 0 || currentMapX >= Map::SIZE || currentMapY < 0 || currentMapY >= Map::SIZE)
                break;

            const Map::Cell& cell = map.get(currentMapX, currentMapY);
            if (cell.type == Map::TileType::Wall && cell.height > 0.0f) {
                float dist = (hitSide == 0) ? currentSideDistX - deltaDistX : currentSideDistY - deltaDistY;
                hits.push_back({ dist, cell.height, hitSide });

                // Stop if wall is full height
                if (cell.height >= 1.0f)
                    break;
            }
        }

        // If nothing hit, draw ceiling/floor and skip
        if (hits.empty()) {
            zBuffer[x] = 1e6f;
            for (int y = 0; y < screenH/2; y++) pixels[y*screenW + x] = 0xFF202040; // ceiling
            for (int y = screenH/2; y < screenH; y++) pixels[y*screenW + x] = 0xFF404020; // floor
            continue;
        }

        // Draw from back to front
        std::sort(hits.begin(), hits.end(), [](const WallHit& a, const WallHit& b){ return a.dist > b.dist; });

        int prevDrawEnd = 0;

        for (const WallHit& hit : hits)
        {
            if (hit.dist <= 0.0f) continue;
            zBuffer[x] = hit.dist;

            int fullLineHeight = int(screenH / hit.dist);

            // Compute draw positions
            int drawEnd = (screenH / 2) + (fullLineHeight / 2);                  // bottom
            int drawStart = drawEnd - int(fullLineHeight * hit.height);          // top of wall segment

            // Draw ceiling only for the space above this wall
            for (int y = prevDrawEnd; y < drawStart; y++)
                pixels[y*screenW + x] = 0xFF202040; // ceiling

            // Draw the wall segment
            uint32_t color = (hit.side == 0) ? 0xFF00AAFF : 0xFF0055FF;
            drawVerticalLine(pixels, screenW, screenH, x, drawStart, drawEnd, color);

            prevDrawEnd = drawEnd; // track bottom for next segment
        }

        // Fill floor below last wall
        for (int y = prevDrawEnd; y < screenH; y++)
            pixels[y*screenW + x] = 0xFF404020;
    }
}

