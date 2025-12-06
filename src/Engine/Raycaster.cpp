#include "Raycaster.h"

// Gives access to trig functions cos(), sin(), etc.
#include <cmath>

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

        int hit = 0;
        int side = 0;

        // initial step
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (player.x - mapX) * deltaDistX;
        } else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - player.x) * deltaDistX;
        }

        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (player.y - mapY) * deltaDistY;
        } else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - player.y) * deltaDistY;
        }

        // DDA
        while (!hit) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (mapX < 0 || mapX >= Map::SIZE ||
                mapY < 0 || mapY >= Map::SIZE)
            {
                hit = 1;
                break;
            }

            if (map.data[mapX][mapY] == 1)
                hit = 1;
        }

        float perpWallDist;
        if (side == 0)
            perpWallDist = sideDistX - deltaDistX;
        else
            perpWallDist = sideDistY - deltaDistY;

        if (perpWallDist <= 0) perpWallDist = 1e-4f;

        zBuffer[x] = perpWallDist;

        int lineHeight = int(screenH / perpWallDist);

        int drawStart = -lineHeight / 2 + screenH / 2;
        if (drawStart < 0) drawStart = 0;

        int drawEnd = lineHeight / 2 + screenH / 2;
        if (drawEnd >= screenH) drawEnd = screenH - 1;

        // fill ceiling
        for (int y = 0; y < drawStart; y++)
            pixels[y * screenW + x] = 0xFF202040;

        // wall
        uint32_t color = (side == 0) ? 0xFF0000FF : 0xFF000088;
        for (int y = drawStart; y < drawEnd; y++)
            pixels[y * screenW + x] = color;

        // fill floor
        for (int y = drawEnd; y < screenH; y++)
            pixels[y * screenW + x] = 0xFF404020;
    }
}

