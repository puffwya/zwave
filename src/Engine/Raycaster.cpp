#include "Raycaster.h"

// Gives access to trig functions cos(), sin(), etc.
#include <cmath>

void Raycaster::render(uint32_t* pixels, int screenW, int screenH, const Player& player, const Map& map, float* zBuffer)
{
    for (int x = 0; x < screenW; x++) {

        // calculate camera plane X coordinate (-1 to 1)
        float cameraX = 2.0f * x / float(screenW) - 1.0f;

        // ray direction
        float rayDirX = cos(player.angle) + cameraX * -sin(player.angle);
        float rayDirY = sin(player.angle) + cameraX *  cos(player.angle);

        int mapX = int(player.x);
        int mapY = int(player.y);

        float sideDistX, sideDistY;

        float deltaDistX = fabs(1 / rayDirX);
        float deltaDistY = fabs(1 / rayDirY);

        float perpWallDist;
        int stepX, stepY;

        int hit = 0;
        int side = 0;

        // initial step direction + distances
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

        // DDA loop
        while (hit == 0) {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            } else {
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }

            if (mapX < 0 || mapX >= Map::SIZE || mapY < 0 || mapY >= Map::SIZE) {
                hit = 1;
                perpWallDist = 1e6f;
                break;
            }

            if (map.data[mapX][mapY] == 1)
                hit = 1;
        }

        if (hit && perpWallDist > 0 && perpWallDist < 1e6f) {
            if (side == 0) perpWallDist = sideDistX - deltaDistX;
            else           perpWallDist = sideDistY - deltaDistY;
        }

        if (!(perpWallDist > 0)) perpWallDist = 1e6f;

        zBuffer[x] = perpWallDist;

        int lineHeight = int(screenH / perpWallDist);

        int drawStart = -lineHeight / 2 + screenH / 2;

        if (drawStart < 0) drawStart = 0;

        int drawEnd = lineHeight / 2 + screenH / 2;

        if (drawEnd >= screenH) drawEnd = screenH - 1;

        // simple color shading
        uint32_t color = side == 0 ? 0xFF0000FF : 0xFF000088;
        for (int y = drawStart; y < drawEnd; y++)
            pixels[y * screenW + x] = color;

        // perpendicular distance to avoid fisheye
        if (side == 0)
            perpWallDist = sideDistX - deltaDistX;
        else
            perpWallDist = sideDistY - deltaDistY;

        // draw the vertical slice
        for (int y = drawStart; y < drawEnd; y++) {
            pixels[y * screenW + x] = color;
        }

        

    }
}

