#include "MapToSegments.h"
#include "Map.h"
#include <cmath>
#include <algorithm>
#include <iostream>

// Helper: check if neighbor is a wall
static inline bool isWall(const Map& map, int x, int y) {
    if (x < 0 || y < 0 || x >= Map::SIZE || y >= Map::SIZE) return false;
    return map.get(x,y).type == Map::TileType::Wall;
}

std::vector<GridSegment> buildSegmentsFromGrid(const Map& map)
{
    std::vector<GridSegment> edges;

    // For each tile that is a wall, produce edges where neighbor is not a wall.
    for (int y = 0; y < Map::SIZE; ++y) {
        for (int x = 0; x < Map::SIZE; ++x) {
            const auto& cell = map.get(x, y);
            if (cell.type != Map::TileType::Wall) continue;

            // Top edge: north
            {
                bool neigh = isWall(map, x, y-1);
                if (!neigh || map.get(x,y-1).height > 0.0f) {
                    float frontH = cell.height;
                    float backH  = neigh ? map.get(x, y-1).height : 0.0f;

                    edges.push_back({
                        {float(x),   float(y)},
                        {float(x+1), float(y)},
                        x, y, frontH, backH, GridSegment::Dir::North
                    });
                }
            }

            // Bottom edge: south
            {
                bool neigh = isWall(map, x, y+1);
                if (!neigh || map.get(x,y+1).height > 0.0f) {
                    float frontH = cell.height;
                    float backH  = neigh ? map.get(x, y+1).height : 0.0f;

                    edges.push_back({
                        {float(x+1), float(y+1)},
                        {float(x),   float(y+1)},
                        x, y, frontH, backH, GridSegment::Dir::South
                    });
                }
            }

            // Left edge: west
            {
                bool neigh = isWall(map, x-1, y);
                if (!neigh || map.get(x-1,y).height > 0.0f) {
                    float frontH = cell.height;
                    float backH  = neigh ? map.get(x-1, y).height : 0.0f;

                    edges.push_back({
                        {float(x),   float(y+1)},
                        {float(x),   float(y)},
                        x, y, frontH, backH, GridSegment::Dir::West
                    });
                }
            }

            // Right edge: east
            {
                bool neigh = isWall(map, x+1, y);
                if (!neigh || map.get(x+1,y).height > 0.0f) {
                    float frontH = cell.height;
                    float backH  = neigh ? map.get(x+1, y).height : 0.0f;

                    edges.push_back({
                        {float(x+1), float(y)},
                        {float(x+1), float(y+1)},
                        x, y, frontH, backH, GridSegment::Dir::East
                    });
                }
            }
        }
    }

    // Split into horizontal & vertical
    std::vector<GridSegment> horiz, vert;
    for (auto& e : edges) {
        if (fabs(e.a.y - e.b.y) < 1e-6f) horiz.push_back(e);
        else vert.push_back(e);
    }

    auto mergeSegments = [&](std::vector<GridSegment>& list, bool horizontal) {
        std::vector<GridSegment> out;

        while (!list.empty()) {
            GridSegment cur = list.back();
            list.pop_back();

            bool merged = true;
            while (merged) {
                merged = false;

                for (auto it = list.begin(); it != list.end(); ++it) {
                    if (it->tileX != cur.tileX || it->tileY != cur.tileY)
                        continue;

                    if (horizontal) {
                        if (fabs(cur.a.y - it->a.y) < 1e-6f) {
                            float a0 = std::min(cur.a.x, cur.b.x);
                            float a1 = std::max(cur.a.x, cur.b.x);
                            float b0 = std::min(it->a.x, it->b.x);
                            float b1 = std::max(it->a.x, it->b.x);

                            if (b0 <= a1+1e-6f && b1 >= a0-1e-6f) {
                                float minx = std::min(a0, b0);
                                float maxx = std::max(a1, b1);
                                cur.a.x = minx; cur.b.x = maxx;
                                cur.a.y = cur.b.y = it->a.y;

                                list.erase(it);
                                merged = true;
                                break;
                            }
                        }
                    } else { // vertical
                        if (fabs(cur.a.x - it->a.x) < 1e-6f) {
                            float a0 = std::min(cur.a.y, cur.b.y);
                            float a1 = std::max(cur.a.y, cur.b.y);
                            float b0 = std::min(it->a.y, it->b.y);
                            float b1 = std::max(it->a.y, it->b.y);

                            if (b0 <= a1+1e-6f && b1 >= a0-1e-6f) {
                                float miny = std::min(a0, b0);
                                float maxy = std::max(a1, b1);
                                cur.a.y = miny; cur.b.y = maxy;
                                cur.a.x = cur.b.x = it->a.x;

                                list.erase(it);
                                merged = true;
                                break;
                            }
                        }
                    }
                }
            }
            out.push_back(cur);
        }
        return out;
    };

    auto mergedH = mergeSegments(horiz, true);
    auto mergedV = mergeSegments(vert, false);

    std::vector<GridSegment> result;
    result.reserve(mergedH.size() + mergedV.size());
    for (auto& s : mergedH) result.push_back(s);
    for (auto& s : mergedV) result.push_back(s);

    return result;
}

std::vector<TopSurface> generateTopSurfaces(const std::vector<GridSegment>& segments, const Map& map) {
    std::vector<TopSurface> tops;

    for (const auto& seg : segments) {
        // Only partial walls
        if (seg.frontHeight < 1.0f) {

            // Optional: only create top if the neighbor is lower
            int nx = seg.tileX;
            int ny = seg.tileY;

            float neighborHeight = 0.0f;

            switch (seg.dir) {
                case GridSegment::Dir::North: neighborHeight = map.get(nx, ny-1).height; break;
                case GridSegment::Dir::South: neighborHeight = map.get(nx, ny+1).height; break;
                case GridSegment::Dir::West:  neighborHeight = map.get(nx-1, ny).height; break;
                case GridSegment::Dir::East:  neighborHeight = map.get(nx+1, ny).height; break;
            }

            // Only add if neighbor is lower or empty
            if (seg.frontHeight > neighborHeight) {
                tops.push_back({ seg.a, seg.b, seg.frontHeight });
            }
        }
    }

    return tops;
}

