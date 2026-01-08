#include "DoomRenderer.h"
#include <cmath>
#include <algorithm>
#include <cassert>
#include <limits>
#include <iostream>

// constants
static constexpr float EPS = 1e-6f;
static constexpr float WALL_WORLD_HEIGHT = 1.0f; // world units for a full-height wall
static constexpr uint32_t COLOR_SIDE_X = 0xFF00AAFF;
static constexpr uint32_t COLOR_SIDE_Y = 0xFF0055FF;
static constexpr uint32_t COLOR_TOP    = 0xFF808080;

SpriteRenderer spriteRenderer;

DoomRenderer::DoomRenderer(const std::vector<GridSegment>& segments,
                           std::unique_ptr<BSPNode> bspRoot)
    : m_segments(segments), m_bspRoot(std::move(bspRoot))
{
}

// small inline draws a vertical column segment
void DoomRenderer::drawSegmentColumnSolid(uint32_t* pixels, int screenW, int screenH,
                                          int sx, int drawStart, int drawEnd, uint32_t color)
{
    if (sx < 0 || sx >= screenW) return;

    // Swap if inverted
    if (drawStart > drawEnd) std::swap(drawStart, drawEnd);

    // Clamp to screen bounds
    drawStart = std::clamp(drawStart, 0, screenH - 1);
    drawEnd   = std::clamp(drawEnd, 0, screenH - 1);

    for (int y = drawStart; y <= drawEnd; ++y) {
        pixels[y * screenW + sx] = color;
    }
}

void DoomRenderer::renderWorldTileRasterized(uint32_t* pixels, float* zBuffer, int screenW, int screenH,
                                             const Player& player,
                                             float wx, float wy, float sizeWorld, float tileHeight,
                                             uint32_t color, const Map& map)
{
    if (tileHeight == 1.0f) return; // no need to render full-height wall tops

    const float minX = wx;
    const float maxX = wx + sizeWorld;
    const float minY = wy;
    const float maxY = wy + sizeWorld;

    const float sa = std::sin(player.angle);
    const float ca = std::cos(player.angle);

    const float cx = screenW * 0.5f;
    const float cy = screenH * 0.5f;
    const float EPS = 1e-6f;

    for (int sx = 0; sx < screenW; ++sx) {
        float s = (float(sx) - cx) / cx;

        // Camera-space direction
        float dirx = s * (-sa) + ca;
        float diry = s * ( ca) + sa;

        // Ray origin
        float ox = player.x;
        float oy = player.y;

        // 2D AABB slab test
        float tmin = -INFINITY, tmax = INFINITY;

        // X slab
        if (fabs(dirx) >= EPS) {
            float tx1 = (minX - ox) / dirx;
            float tx2 = (maxX - ox) / dirx;
            if (tx1 > tx2) std::swap(tx1, tx2);
            tmin = std::max(tmin, tx1);
            tmax = std::min(tmax, tx2);
        } else if (ox < minX || ox > maxX) continue;

        // Y slab
        if (fabs(diry) >= EPS) {
            float ty1 = (minY - oy) / diry;
            float ty2 = (maxY - oy) / diry;
            if (ty1 > ty2) std::swap(ty1, ty2);
            tmin = std::max(tmin, ty1);
            tmax = std::min(tmax, ty2);
        } else if (oy < minY || oy > maxY) continue;

        if (tmax < tmin) continue; // miss

        float t_enter = std::max(tmin, 1e-5f);
        if (tmax <= 1e-5f) continue; // fully behind camera

        // Project vertical position
        float screenY_enter = cy - ((tileHeight - player.z) * screenH / t_enter);
        float screenY_exit  = cy - ((tileHeight - player.z) * screenH / tmax);

        int yTop = std::max(0, int(std::ceil(std::min(screenY_enter, screenY_exit))));
        int yBottom = std::min(screenH - 1, int(std::floor(std::max(screenY_enter, screenY_exit))));

        if (yBottom < 0 || yTop >= screenH) continue;

        // Z-buffer test (horizontal spans do NOT update zBuffer)
        if (t_enter >= zBuffer[sx]) continue;

        // Draw vertical span
        uint32_t* px = pixels + yTop * screenW + sx;
        for (int y = yTop; y <= yBottom; ++y) {
            *px = color;
            px += screenW;
        }
    }
}

// Project (wx,wy) into camera space relative to player. Returns camera-space X and forward (Y).
bool DoomRenderer::projectPointToCamera(float wx, float wy, const Player& player,
                                       float& out_camX, float& out_camY)
{
    float ca = std::cos(player.angle);
    float sa = std::sin(player.angle);
    float relX = wx - player.x;
    float relY = wy - player.y;
    // camera right = (-sin, cos), forward = (cos, sin)
    out_camX = relX * (-sa) + relY * ca; // cam X
    out_camY = relX * ca + relY * sa;    // cam Y (forward)
    return out_camY > 1e-5f; // in front of camera
}

// Rasterize a single segment (as vertical wall) in screen columns
// approximate per-column depth by linear interpolation between endpoints' camY
// seg endpoints: seg.a (wx,wy) -> seg.b
void DoomRenderer::rasterizeSegment(const GridSegment& seg, int mapTileX, int mapTileY,
                                     uint32_t* pixels, int screenW, int screenH,
                                     const Player& player, const Map& map, float* zBuffer)
{
    // Project endpoints to camera space
    float a_camX, a_camY, b_camX, b_camY;
    bool aFront = projectPointToCamera(seg.a.x, seg.a.y, player, a_camX, a_camY);
    bool bFront = projectPointToCamera(seg.b.x, seg.b.y, player, b_camX, b_camY);

    if (!aFront && !bFront) return;

    const float clipNear = 0.001f;
    if (!aFront || !bFront) {
        // Clip segment against near plane
        float t = (clipNear - a_camY) / (b_camY - a_camY);
        t = std::clamp(t, 0.0f, 1.0f);
        if (!aFront) {
            float newX = seg.a.x + t * (seg.b.x - seg.a.x);
            float newY = seg.a.y + t * (seg.b.y - seg.a.y);
            projectPointToCamera(newX, newY, player, a_camX, a_camY);
        } else {
            float newX = seg.a.x + t * (seg.b.x - seg.a.x);
            float newY = seg.a.y + t * (seg.b.y - seg.a.y);
            projectPointToCamera(newX, newY, player, b_camX, b_camY);
        }
    }

    // Project to screen X
    float sxA = (a_camX / a_camY) * (screenW * 0.5f) + (screenW * 0.5f);
    float sxB = (b_camX / b_camY) * (screenW * 0.5f) + (screenW * 0.5f);

    // Skip if entirely off-screen horizontally
    if ((sxA < -screenW && sxB < -screenW) || (sxA > 2*screenW && sxB > 2*screenW)) return;

    int x0 = std::clamp(int(std::floor(std::min(sxA, sxB))), 0, screenW - 1);
    int x1 = std::clamp(int(std::ceil (std::max(sxA, sxB))), 0, screenW - 1);
    if (x1 < x0) std::swap(x0, x1);

    // Wall heights for this tile
    float tileH = map.get(mapTileX, mapTileY).height;
    float floorZ = (tileH < 0.0f) ? tileH : 0.0f;
    float ceilZ  = (tileH < 0.0f) ? 0.0f : tileH;

    // Project top/bottom for endpoints
    float a_sy_floor   = (screenH * 0.5f) - (floorZ  - player.z) * (screenH / a_camY);
    float a_sy_ceiling = (screenH * 0.5f) - (ceilZ   - player.z) * (screenH / a_camY);
    float b_sy_floor   = (screenH * 0.5f) - (floorZ  - player.z) * (screenH / b_camY);
    float b_sy_ceiling = (screenH * 0.5f) - (ceilZ   - player.z) * (screenH / b_camY);

    for (int sx = x0; sx <= x1; ++sx) {
        float t = (fabs(sxB - sxA) > 1e-6f) ? (sx - sxA) / (sxB - sxA) : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);

        float depth = a_camY + t * (b_camY - a_camY);
        if (depth <= 0.0001f) continue;

        float colFloorY   = a_sy_floor   + t * (b_sy_floor   - a_sy_floor);
        float colCeilY    = a_sy_ceiling + t * (b_sy_ceiling - a_sy_ceiling);

        int drawStart = std::max(0, int(std::ceil(colCeilY)));
        int drawEnd   = std::min(screenH - 1, int(std::floor(colFloorY)));
        if (drawEnd < 0 || drawStart >= screenH) continue;

        uint32_t color = (fabs(seg.a.x - seg.b.x) > fabs(seg.a.y - seg.b.y)) ? COLOR_SIDE_X : COLOR_SIDE_Y;

        drawSegmentColumnSolid(pixels, screenW, screenH, sx, drawStart, drawEnd, color);

        if (tileH < 0.0f) {
            continue;
        }
        zBuffer[sx] = depth;
    }
}

// side test: positive if P is left of AB
float DoomRenderer::sideOfLine(float ax, float ay, float bx, float by, float px, float py)
{
    float vx = bx - ax, vy = by - ay;
    float wx = px - ax, wy = py - ay;
    return vx * wy - vy * wx;
}

// traverse BSP front to back relative to player's position 
void DoomRenderer::traverseBSP(
    const BSPNode* node,
    const Player& player,
    uint32_t* pixels,
    int screenW,
    int screenH,
    const Map& map,
    float* zBuffer,
    uint8_t* tileDrawn
) {
    if (!node) return;

    // Determine traversal order
    float side = sideOfLine(
        node->splitA.x, node->splitA.y,
        node->splitB.x, node->splitB.y,
        player.x, player.y
    );

    const BSPNode* first  = (side > 0.0f) ? node->front.get() : node->back.get();
    const BSPNode* second = (side > 0.0f) ? node->back.get()  : node->front.get();

    // Traverse far side first
    if (second)
        traverseBSP(second, player, pixels, screenW, screenH, map, zBuffer, tileDrawn);

    // Pass 1: vertical walls only
    for (const auto& seg : node->onPlane) {
        int tx = seg.tileX;
        int ty = seg.tileY;

        if (tx < 0 || tx >= Map::SIZE || ty < 0 || ty >= Map::SIZE)
            continue;

        const Map::Cell& cell = map.get(tx, ty);
        float h = cell.height;

        // Draw vertical walls (normal or pit)
        if (h != 0.0f) {
            rasterizeSegment(seg, tx, ty, pixels, screenW, screenH, player, map, zBuffer);
        }
    }

    // Pass 2: floors, pits, wall tops
    for (const auto& seg : node->onPlane) {
        int tx = seg.tileX;
        int ty = seg.tileY;

        if (tx < 0 || tx >= Map::SIZE || ty < 0 || ty >= Map::SIZE)
            continue;

        bool notNearPit = true;

        int idx = tx + ty * Map::SIZE;
        if (tileDrawn[idx])
            continue;
        if (map.get(tx+1,ty).height < 0 || map.get(tx,ty+1).height < 0 || map.get(tx,ty-1).height < 0 || map.get(tx-1,ty).height < 0 || map.get(tx+1,ty+1).height < 0 || map.get(tx-1,ty-1).height < 0) {
            tileDrawn[idx] = false;
            notNearPit = false;
        }
        else if (map.get(tx+2,ty).height < 0 || map.get(tx,ty+2).height < 0 || map.get(tx,ty-2).height < 0 || map.get(tx-2,ty).height < 0 || map.get(tx+2,ty+2).height < 0 || map.get(tx-2,ty-2).height < 0) {
            tileDrawn[idx] = false;
            notNearPit = false;
        }
        else {
            tileDrawn[idx] = true;
        }

        const Map::Cell& cell = map.get(tx, ty);
        float h = cell.height;

        if (h == 0.0f && notNearPit) {
            continue;
        }
        else if (h < 0.0f) {
            continue;
        }

        uint32_t floorColor;

        if (h < 0.0f) {
            // Pit floor
            floorColor = 0xFF0055FF;
        }
        else if (h > 0.0f && h != WALL_WORLD_HEIGHT) {
            // Wall top
            floorColor = 0xFF0055FF;
        }
        else {
            // Normal flat floor
            floorColor = 0xFF404020;
        }

        renderWorldTileRasterized(
            pixels, zBuffer,
            screenW, screenH,
            player,
            float(tx), float(ty),
            1.0f,
            h,
            floorColor,
            map
        );
    }

    // Traverse near side last
    if (first)
        traverseBSP(first, player, pixels, screenW, screenH, map, zBuffer, tileDrawn);
}

// Main render entry
void DoomRenderer::render(uint32_t* pixels, int screenW, int screenH,
                          const Player& player, Map& map, float* zBuffer, EnemyManager& em)
{
    const uint32_t CEIL_COLOR = 0xFF202040; // World ceiling color (change to texture in the future)

    // Clear framebuffer
    static constexpr uint32_t CLEAR_PIXEL = 0x00000000; // fully transparent black
    std::fill(pixels, pixels + screenW * screenH, CLEAR_PIXEL);

    // Init zBuffer
    for (int x = 0; x < screenW; ++x)
        zBuffer[x] = 1e6f;

    static std::vector<uint8_t> tileDrawn;
    tileDrawn.assign(Map::SIZE * Map::SIZE, 0);

    // Traverse BSP and draw segments front-to-back
    traverseBSP(m_bspRoot.get(), player, pixels, screenW, screenH, map, zBuffer, tileDrawn.data());

    // Fill Ceiling
    for (int y = 0; y < screenH / 2; ++y)
    {
        for (int x = 0; x < screenW; ++x)
        {
            int idx = y * screenW + x;
            if (pixels[idx] == CLEAR_PIXEL)
                pixels[idx] = CEIL_COLOR;
        }
    }

    // fill default floor (bottom half of screen)
    for (int y = screenH / 2; y < screenH; ++y)
    {
        for (int x = 0; x < screenW; ++x)
        {
            int idx = y * screenW + x;
            if (pixels[idx] == CLEAR_PIXEL)
                pixels[idx] = 0xFF404020; // default floor
        }
    }

    // Draw enemies                  
    spriteRenderer.renderEnemies(pixels, screenW, screenH, em, player, zBuffer, map);
}
