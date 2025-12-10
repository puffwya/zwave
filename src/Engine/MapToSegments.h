#pragma once
#include <vector>
#include "Map.h"

struct Vec2 { float x, y; };

struct GridSegment {
    Vec2 a, b;      // endpoints in world coordinates
    int tileX, tileY;  

    float frontHeight;   // height of this wall tile (map.get(tileX,tileY).height)
    float backHeight;    // height of neighbor tile (0 if empty, or whatever)

    // Optional, but VERY useful for debugging:
    enum class Dir { North, South, West, East } dir;
};

struct TopSurface {
    Vec2 a, b;        // segment endpoints in world space
    float height;   // height of top surface
};

// Produces a compact set of segments from grid walls.
// - map.get(x,y).type == Wall are considered walls.
// - cell size is 1.0 unit per tile (world units).
std::vector<GridSegment> buildSegmentsFromGrid(const Map& map);

std::vector<TopSurface> generateTopSurfaces(const std::vector<GridSegment>& segments, const Map& map);
