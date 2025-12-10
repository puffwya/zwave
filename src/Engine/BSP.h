#ifndef BSP_H
#define BSP_H

#include <vector>
#include <memory>
#include "MapToSegments.h" // provides GridSegment

using Segment = GridSegment; // reuse your GridSegment type

struct BSPNode {
    // splitter line defined by two points (in world coords)
    Vec2 splitA;
    Vec2 splitB;

    // segments that lie exactly on the splitter (collinear or touching)
    std::vector<Segment> onPlane;

    // child nodes: front = side where cross > 0, back = side where cross < 0
    std::unique_ptr<BSPNode> front;
    std::unique_ptr<BSPNode> back;
};

/// Build a BSP tree from a list of segments. Returns root node (nullptr if no segments).
std::unique_ptr<BSPNode> buildBSP(const std::vector<Segment>& segments);

/// Collect subsectors (leaf lists). Each subsector is a vector<Segment>.
void collectSubsectors(const std::unique_ptr<BSPNode>& root,
                       std::vector<std::vector<Segment>>& outSubsectors);

#endif // BSP_H

