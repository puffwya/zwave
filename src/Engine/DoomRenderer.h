#ifndef DOOM_RENDERER_H
#define DOOM_RENDERER_H

#include <vector>
#include <cstdint>
#include "BSP.h"         // uses Segment type (GridSegment)
#include "Map.h"
#include "Player.h"

class DoomRenderer {
public:
    // Provide segments and BSP root to the renderer (built by earlier steps).
    DoomRenderer(const std::vector<GridSegment>& segments,
                 std::unique_ptr<BSPNode> bspRoot);

    // Render into pixels buffer. zBuffer must be length screenW.
    void render(uint32_t* pixels, int screenW, int screenH,
                const Player& player, const Map& map, float* zBuffer);

private:
    std::vector<GridSegment> m_segments;
    std::unique_ptr<BSPNode> m_bspRoot;

    static void renderWorldTileRasterized(uint32_t* pixels, float* zBuffer, int screenW, int screenH,
                                      const Player& player,
                                      float wx, float wy, float sizeWorld, float tileHeight,
                                      uint32_t color, float* spanDepth, const Map& map);

    // helper drawing
    void drawSegmentColumnSolid(uint32_t* pixels, int screenW, int screenH,
                                int sx, int drawStart, int drawEnd, uint32_t color);

    // Project endpoints to camera space; returns false if completely behind camera
    bool projectPointToCamera(float wx, float wy, const Player& player,
                              float& out_camX, float& out_camY);

    // Rasterize segment between two projected endpoints (screen Xs / depths)
    void rasterizeSegment(const GridSegment& seg, int mapTileX, int mapTileY,
                          uint32_t* pixels, int screenW, int screenH,
                          const Player& player, const Map& map, float* zBuffer, float* spanDepth, float* spanInvDepth);

    // BSP traversal
    void traverseBSP(const BSPNode* node, const Player& player,
                     uint32_t* pixels, int screenW, int screenH,
                     const Map& map, float* zBuffer);

    // small helpers
    static float sideOfLine(float ax, float ay, float bx, float by, float px, float py);
};

#endif // DOOM_RENDERER_H

