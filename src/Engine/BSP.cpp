#include "BSP.h"
#include <algorithm>
#include <cmath>
#include <cassert>

// small epsilon
static constexpr float EPS = 1e-6f;

static inline float cross(const Vec2& a, const Vec2& b) {
    return a.x * b.y - a.y * b.x;
}
static inline Vec2 sub(const Vec2& a, const Vec2& b) { return { a.x - b.x, a.y - b.y }; }
static inline Vec2 add(const Vec2& a, const Vec2& b) { return { a.x + b.x, a.y + b.y }; }
static inline Vec2 mul(const Vec2& a, float s) { return { a.x * s, a.y * s }; }

// side test: positive = left of AB, negative = right of AB, zero = collinear
static float sideOfLine(const Vec2& A, const Vec2& B, const Vec2& P) {
    return cross(sub(B, A), sub(P, A));
}

// Solve intersection of segments p0-p1 and p2-p3
// Returns true if intersects (including collinear overlapping), and t,u in [0,1] param coords for p0..p1 and p2..p3
static bool intersectSegmentSegment(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3,
                                    float &outT, float &outU)
{
    Vec2 r = sub(p1, p0);
    Vec2 s = sub(p3, p2);
    float rxs = cross(r, s);
    Vec2 diff = sub(p2, p0);
    float qpxr = cross(diff, r);

    if (fabs(rxs) < EPS) {
        // Parallel (collinear or parallel non-intersecting)
        return false;
    }
    outT = cross(diff, s) / rxs;
    outU = qpxr / rxs;
    // intersection occurs when 0 <= t <= 1 and 0 <= u <= 1
    return (outT >= -EPS && outT <= 1.0f + EPS && outU >= -EPS && outU <= 1.0f + EPS);
}

// Clip (split) segment S by splitter line AB if it crosses.
// If segment straddles the line, returns two segments split at intersection.
// If it lies fully on one side, returns original in that side vector.
// If collinear, classify as on-plane by caller.
static void splitSegmentIfCross(const Segment& seg, const Vec2& A, const Vec2& B,
                                std::vector<Segment>& outFront, std::vector<Segment>& outBack,
                                std::vector<Segment>& outOnPlane)
{
    Vec2 p0{seg.a.x, seg.a.y};
    Vec2 p1{seg.b.x, seg.b.y};

    float s0 = sideOfLine(A, B, p0);
    float s1 = sideOfLine(A, B, p1);

    // collinear / on-plane
    if (fabs(s0) < EPS && fabs(s1) < EPS) {
        outOnPlane.push_back(seg);
        return;
    }

    // same side -> push to front or back
    if (s0 > EPS && s1 > EPS) { outFront.push_back(seg); return; }
    if (s0 < -EPS && s1 < -EPS) { outBack.push_back(seg); return; }

    // Segment crosses the line -> split at intersection point
    float t = 0.0f, u = 0.0f;
    if (!intersectSegmentSegment(p0, p1, A, B, t, u)) {
        // numerical or edge cases: if one endpoint is almost on the line, classify accordingly
        if (fabs(s0) < EPS) { // p0 on line
            if (s1 > 0) outFront.push_back(seg);
            else outBack.push_back(seg);
            return;
        }
        if (fabs(s1) < EPS) {
            if (s0 > 0) outFront.push_back(seg);
            else outBack.push_back(seg);
            return;
        }
        // otherwise fallback: push to the side of midpoint
        Vec2 mid = mul(add(p0,p1), 0.5f);
        float sm = sideOfLine(A,B,mid);
        if (sm > 0) outFront.push_back(seg); else outBack.push_back(seg);
        return;
    }

    // create intersection point Pi = p0 + t*(p1-p0)
    Vec2 dir = sub(p1, p0);
    Vec2 Pi = add(p0, mul(dir, t));

    // Build two segments
    Segment sA{ {p0.x, p0.y}, {Pi.x, Pi.y}, seg.tileX, seg.tileY };
    Segment sB{ {Pi.x, Pi.y}, {p1.x, p1.y}, seg.tileX, seg.tileY };

    // determine which portion goes to which side by checking midpoints
    Vec2 midA = mul(add(p0, Pi), 0.5f);
    Vec2 midB = mul(add(Pi, p1), 0.5f);

    float sideA = sideOfLine(A, B, midA);
    float sideB = sideOfLine(A, B, midB);

    if (sideA > 0) outFront.push_back(sA); else if (fabs(sideA) < EPS) outOnPlane.push_back(sA); else 
outBack.push_back(sA);
    if (sideB > 0) outFront.push_back(sB); else if (fabs(sideB) < EPS) outOnPlane.push_back(sB); else 
outBack.push_back(sB);
}

// Choose a splitter segment index. Current heuristic: pick segment with longest length (reduce splits)
static int chooseSplitterIndex(const std::vector<Segment>& segs)
{
    if (segs.empty()) return -1;
    int best = 0;
    float bestLen = 0.0f;
    for (size_t i = 0; i < segs.size(); ++i) {
        float dx = segs[i].a.x - segs[i].b.x;
        float dy = segs[i].a.y - segs[i].b.y;
        float len = dx*dx + dy*dy;
        if (len > bestLen) { bestLen = len; best = (int)i; }
    }
    return best;
}

static std::unique_ptr<BSPNode> buildBSPRecursive(const std::vector<Segment>& input)
{
    if (input.empty()) return nullptr;

    // allocate node
    auto node = std::make_unique<BSPNode>();

    // choose splitter
    int idx = chooseSplitterIndex(input);
    if (idx < 0) return nullptr;
    Segment splitter = input[idx];

    // define split line A->B
    node->splitA = { splitter.a.x, splitter.a.y };
    node->splitB = { splitter.b.x, splitter.b.y };

    // containers for classification
    std::vector<Segment> frontList;
    std::vector<Segment> backList;
    std::vector<Segment> onPlaneList;

    // classify all segments (including the splitter itself as onPlane)
    for (size_t i = 0; i < input.size(); ++i) {
        if ((int)i == idx) {
            onPlaneList.push_back(input[i]);
            continue;
        }
        splitSegmentIfCross(input[i], node->splitA, node->splitB, frontList, backList, onPlaneList);
    }

    node->onPlane = std::move(onPlaneList);

    // Recurse
    node->front = buildBSPRecursive(frontList);
    node->back  = buildBSPRecursive(backList);

    return node;
}

std::unique_ptr<BSPNode> buildBSP(const std::vector<Segment>& segments)
{
    // filter trivial degenerate segments (zero length)
    std::vector<Segment> good;
    good.reserve(segments.size());
    for (const auto &s : segments) {
        float dx = s.a.x - s.b.x;
        float dy = s.a.y - s.b.y;
        if (fabs(dx) < EPS && fabs(dy) < EPS) continue;
        good.push_back(s);
    }
    return buildBSPRecursive(good);
}

static void collectLeavesRecursive(const std::unique_ptr<BSPNode>& node,
                                   std::vector<std::vector<Segment>>& out)
{
    if (!node) return;
    if (!node->front && !node->back) {
        // leaf -> create subsector using onPlane segments (could be empty)
        out.emplace_back(node->onPlane);
        return;
    }
    if (node->front) collectLeavesRecursive(node->front, out);
    if (node->back)  collectLeavesRecursive(node->back, out);
}

void collectSubsectors(const std::unique_ptr<BSPNode>& root,
                       std::vector<std::vector<Segment>>& outSubsectors)
{
    outSubsectors.clear();
    collectLeavesRecursive(root, outSubsectors);
}

