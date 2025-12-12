#include "DoomRenderer.h"
#include <cmath>
#include <algorithm>
#include <cassert>
#include <limits>

// constants
static constexpr float EPS = 1e-6f;
static constexpr float WALL_WORLD_HEIGHT = 1.0f; // world units for a full-height wall
static constexpr uint32_t COLOR_SIDE_X = 0xFF00AAFF;
static constexpr uint32_t COLOR_SIDE_Y = 0xFF0055FF;
static constexpr uint32_t COLOR_TOP    = 0xFF808080;

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
    if (drawStart < 0) drawStart = 0;
    if (drawEnd >= screenH) drawEnd = screenH - 1;
    for (int y = drawStart; y <= drawEnd; ++y) {
        pixels[y * screenW + sx] = color;
    }
}

void DoomRenderer::renderWorldTileRasterized(uint32_t* pixels, float* zBuffer, int screenW, int screenH,
                                      const Player& player,
                                      float wx, float wy, float sizeWorld, float tileHeight,
                                      uint32_t color, float* spanDepth)
{
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
        // normalized screen coordinate in [-1,1]
        float s = (float(sx) - cx) / cx;

        // camera-space direction in world XY for this column (dirx, diry)
        // right  = (-sin, cos), forward = (cos, sin)
        float dirx = s * (-sa) + ca;
        float diry = s * ( ca) + sa;

        // ray origin in world XY
        float ox = player.x;
        float oy = player.y;

        // ray-AABB slab test in 2D
        float tmin = -INFINITY;
        float tmax =  INFINITY;

        // X slab
        if (fabs(dirx) < EPS) {
            if (ox < minX || ox > maxX) continue; // parallel and outside -> no hit
        } else {
            float tx1 = (minX - ox) / dirx;
            float tx2 = (maxX - ox) / dirx;
            if (tx1 > tx2) std::swap(tx1, tx2);
            tmin = std::max(tmin, tx1);
            tmax = std::min(tmax, tx2);
        }

        // Y slab
        if (fabs(diry) < EPS) {
            if (oy < minY || oy > maxY) continue;
        } else {
            float ty1 = (minY - oy) / diry;
            float ty2 = (maxY - oy) / diry;
            if (ty1 > ty2) std::swap(ty1, ty2);
            tmin = std::max(tmin, ty1);
            tmax = std::min(tmax, ty2);
        }

        if (tmax < tmin) continue; // miss

        // want forward hits only
        float t_enter = tmin;
        float t_exit  = tmax;
        if (t_exit <= 1e-5f) continue;       // fully behind camera
        if (t_enter < 1e-5f) t_enter = 1e-5f;

        // camY ~ forward distance; in our ray dir construction camY == t (proportional)
        float camY_enter = t_enter;
        float camY_exit  = t_exit;

        // Project vertical position for plane height at both t values:
        float screenY_enter = cy - ((tileHeight - player.z) * (float(screenH) / camY_enter));
        float screenY_exit  = cy - ((tileHeight - player.z) * (float(screenH) / camY_exit));

        int yTop = int(std::ceil(std::min(screenY_enter, screenY_exit)));
        int yBottom = int(std::floor(std::max(screenY_enter, screenY_exit)));

        if (yBottom < 0 || yTop >= screenH) continue;
        if (yTop < 0) yTop = 0;
        if (yBottom >= screenH) yBottom = screenH - 1;

        // Z-buffer test for column: if something nearer already, skip
        if (t_enter >= zBuffer[sx]) continue;

        // HACK: stores the depth of this horizontal span in the per-column array so that walls that should be 
        // behind it do not appear in front when horizontal spans zBuffer is set to INFINITY...yikes
        spanDepth[sx] = t_exit;

        // Fill vertical span for this column
        uint32_t* px = pixels + yTop * screenW + sx;
        for (int y = yTop; y <= yBottom; ++y) {
            *px = color;
            px += screenW;
        }

        // update zBuffer to infinity as to not override anything in the column
        zBuffer[sx] = INFINITY;
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

// Rasterize a single segment (as vertical wall) in screen columns.
// We approximate per-column depth by linear interpolation between endpoints' camY.
// seg endpoints: seg.a (wx,wy) -> seg.b
void DoomRenderer::rasterizeSegment(const GridSegment& seg, int mapTileX, int mapTileY,
                                    uint32_t* pixels, int screenW, int screenH,
                                    const Player& player, const Map& map, float* zBuffer, float* spanDepth)
{
    // Project endpoints
    float a_camX, a_camY, b_camX, b_camY;
    bool aFront = projectPointToCamera(seg.a.x, seg.a.y, player, a_camX, a_camY);
    bool bFront = projectPointToCamera(seg.b.x, seg.b.y, player, b_camX, b_camY);

    // If both endpoints are behind camera, skip
    if (!aFront && !bFront) return;

    // If one endpoint is behind, clip the segment against near plane (camY ~= small positive)
    // Simple linear clip T between endpoints when camY crosses tiny threshold
    float clipNear = 0.001f;
    float tClip = 0.0f;
    bool clipped = false;
    if (!aFront && bFront) {
        // find t where camY == clipNear
        float da = b_camY - a_camY;
        if (fabs(da) > EPS) {
            tClip = (clipNear - a_camY) / da;
            tClip = std::clamp(tClip, 0.0f, 1.0f);
            // compute new a as a + tClip*(b-a) in world space
            float newX = seg.a.x + tClip * (seg.b.x - seg.a.x);
            float newY = seg.a.y + tClip * (seg.b.y - seg.a.y);
            // reproject
            projectPointToCamera(newX, newY, player, a_camX, a_camY);
            clipped = true;
        }
    } else if (aFront && !bFront) {
        float da = b_camY - a_camY;
        if (fabs(da) > EPS) {
            tClip = (clipNear - a_camY) / da;
            tClip = std::clamp(tClip, 0.0f, 1.0f);
            float newX = seg.a.x + tClip * (seg.b.x - seg.a.x);
            float newY = seg.a.y + tClip * (seg.b.y - seg.a.y);
            projectPointToCamera(newX, newY, player, b_camX, b_camY);
            clipped = true;
        }
    }

    // Now project to screen X
    float sxA = (a_camX / a_camY) * (screenW * 0.5f) + (screenW * 0.5f);
    float sxB = (b_camX / b_camY) * (screenW * 0.5f) + (screenW * 0.5f);

    // If both screen X are outside and to same side, skip
    if ((sxA < -screenW && sxB < -screenW) || (sxA > 2*screenW && sxB > 2*screenW)) return;

    // Determine integer column range
    int x0 = std::clamp(int(std::floor(std::min(sxA, sxB))), 0, screenW - 1);
    int x1 = std::clamp(int(std::ceil (std::max(sxA, sxB))), 0, screenW - 1);
    if (x1 < x0) std::swap(x0, x1);

    // Get wall heights for this segment from the tile it belongs to
    // We use tile's stored height; if out-of-range, default to full height (1.0)
    float tileH = WALL_WORLD_HEIGHT;
    if (mapTileX >= 0 && mapTileX < Map::SIZE && mapTileY >= 0 && mapTileY < Map::SIZE) {
        const Map::Cell& c = map.get(mapTileX, mapTileY);
        // if your map stores fractional height in c.height, use that. else fallback.
        // clamp
        if (c.height > 0.0f && c.height <= 1.0f) tileH = WALL_WORLD_HEIGHT * c.height;
        else tileH = WALL_WORLD_HEIGHT;
    }

    // vertical positions: bottom at floor (assume floor=0), top at tileH
    float floorZ = 0.0f;
    float ceilingZ = tileH;

    // For endpoints, compute projected Y positions of floor & ceiling
    // sy = screenH/2 - dz * (screenH / camY)   (same formula used earlier)
    float a_sy_floor   = (screenH * 0.5f) - (floorZ  - player.z) * (float(screenH) / a_camY);
    float a_sy_ceiling = (screenH * 0.5f) - (ceilingZ - player.z) * (float(screenH) / a_camY);
    float b_sy_floor   = (screenH * 0.5f) - (floorZ  - player.z) * (float(screenH) / b_camY);
    float b_sy_ceiling = (screenH * 0.5f) - (ceilingZ - player.z) * (float(screenH) / b_camY);

    // For each screen column between x0..x1, interpolate along segment in screen X
    for (int sx = x0; sx <= x1; ++sx) {
        // compute interpolation t along screen X between endpoints
        float t;
        if (fabs(sxB - sxA) > 1e-6f) t = (sx - sxA) / (sxB - sxA);
        else t = 0.0f;

        // clamp t
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        // Interpolate depth (camY) linearly between endpoints
        float depth = a_camY + t * (b_camY - a_camY);
        if (depth <= 0.0001f) continue;

        // Hack: skip wall columns behind horizontal span as their zBuffer is set to infinite
        // Double Hack: skip hack for fractional-height walls
        bool isPartialWall = fabs(seg.frontHeight - seg.backHeight) > 1e-6f && seg.frontHeight < 1.0f;
        if (!(isPartialWall && depth >= spanDepth[sx])) {
            if (depth >= spanDepth[sx]) continue;
        }

        // Z-buffer test: if this depth is farther than zBuffer, skip (behind something previously drawn)
        // if (depth >= zBuffer[sx]) continue;

        // Interpolate top and bottom screen Y for this column
        float colFloorY   = a_sy_floor   + t * (b_sy_floor   - a_sy_floor);
        float colCeilY    = a_sy_ceiling + t * (b_sy_ceiling - a_sy_ceiling);

        // Convert to ints and clamp
        int drawStart = int(std::ceil(colCeilY));
        int drawEnd   = int(std::floor(colFloorY));
        if (drawEnd < 0 || drawStart >= screenH) {
            // nothing visible in column
            // but if top slice visual is needed we could still draw very thin; skip
            continue;
        }
        if (drawStart < 0) drawStart = 0;
        if (drawEnd >= screenH) drawEnd = screenH - 1;

        // choose color by segment side (approx)
        // We have seg.a->seg.b world orientation. Use whether segment mainly aligned x or y for tint.
        uint32_t color = (fabs(seg.a.x - seg.b.x) > fabs(seg.a.y - seg.b.y)) ? COLOR_SIDE_X : COLOR_SIDE_Y;

        // Draw vertical line in this column
        drawSegmentColumnSolid(pixels, screenW, screenH, sx, drawStart, drawEnd, color);

        // Update zBuffer so nearer things will occlude later
        // If is a partial wall set zbuffer to infinity + 1...wtf
        if (isPartialWall) {
            zBuffer[sx] = INFINITY + 1;
        }
        else {
            zBuffer[sx] = depth;
        }
    }
}

// side test: positive if P is left of AB
float DoomRenderer::sideOfLine(float ax, float ay, float bx, float by, float px, float py)
{
    float vx = bx - ax, vy = by - ay;
    float wx = px - ax, wy = py - ay;
    return vx * wy - vy * wx;
}

// traverse BSP front->back relative to player's position
void DoomRenderer::traverseBSP(const BSPNode* node, const Player& player,
                               uint32_t* pixels, int screenW, int screenH,
                               const Map& map, float* zBuffer)
{
    if (!node) return;

    // allocate once per frame / per traverse
    std::vector<float> spanDepth(screenW, INFINITY);

    // determine which side player is on relative to split line
    float side = sideOfLine(node->splitA.x, node->splitA.y, node->splitB.x, node->splitB.y, player.x, player.y);
    // if side > 0 => player on left/front side (we'll treat front as left)
    const BSPNode* first = (side > 0.0f) ? node->front.get() : node->back.get();
    const BSPNode* second = (side > 0.0f) ? node->back.get()  : node->front.get();

    // traverse far side first (second), then draw onPlane segments, then near side (first)
    if (second) traverseBSP(second, player, pixels, screenW, screenH, map, zBuffer);

    // draw all segments lying on the plane (node->onPlane)
    for (const auto& seg : node->onPlane) {
        // --- draw horizontal tile plane for the tile this segment belongs to ---
        // Use tile min-corner world coordinates. Adjust sizeWorld if your tiles are not 1.0 world unit.
        const float sizeWorld = 1.0f;
        int tx = seg.tileX;
        int ty = seg.tileY;

        // fetch tile height from map (uses existing map accessor)
        float tileHeight = WALL_WORLD_HEIGHT; // fallback
        if (tx >= 0 && tx < Map::SIZE && ty >= 0 && ty < Map::SIZE) {
            const Map::Cell& c = map.get(tx, ty);
            tileHeight = c.height; // height in world units
        }

        // choose a top surface color
        const uint32_t TILE_COLOR = 0xFF0055FF; // eventually compute based on sector

        // Render tile AABB anchored at tile corner (tx,ty)
        renderWorldTileRasterized(pixels, zBuffer, screenW, screenH, player,
                                  /*wx*/ float(tx), /*wy*/ float(ty),
                                  /*sizeWorld*/ sizeWorld, /*tileHeight*/ tileHeight,
                                  TILE_COLOR, spanDepth.data());
        // --- end horizontal plane draw ---

        // Now draw the vertical wall segment as before
        rasterizeSegment(seg, seg.tileX, seg.tileY, pixels, screenW, screenH, player, map, zBuffer, spanDepth.data());
    }


    if (first) traverseBSP(first, player, pixels, screenW, screenH, map, zBuffer);
}

// Main render entry
void DoomRenderer::render(uint32_t* pixels, int screenW, int screenH,
                          const Player& player, const Map& map, float* zBuffer)
{
    // Prepare background (ceiling/floor simple fill)
    const uint32_t CEIL_COLOR  = 0xFF202040;
    const uint32_t FLOOR_COLOR = 0xFF404020;

    int floorY = screenH/2;   // raise/lower floor

    for (int x = 0; x < screenW; ++x) {
        for (int y = 0; y < screenH/2; ++y) pixels[y * screenW + x] = CEIL_COLOR;
        for (int y = floorY; y < screenH; ++y) pixels[y * screenW + x] = FLOOR_COLOR;
        zBuffer[x] = 1e6f; // initialize as far away
    }

    // draw horizontal tile at world tile (4,8), size=1.0, at height 0.25
    // renderWorldTileRasterized(pixels, zBuffer, screenW, screenH, player, 5.0f, 8.0f, 1.0f, 0.25f, 0xFF00CC00);

    // Traverse BSP and draw segments front-to-back
    traverseBSP(m_bspRoot.get(), player, pixels, screenW, screenH, map, zBuffer);

    // Note: you can add sprite rendering (sorted by depth) after this, and visplane floor/ceiling rendering if desired.
}

