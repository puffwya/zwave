#pragma once
#include <vector>
#include <string>
#include "MapToSegments.h"

enum class BulletHoleType {
    Pistol,
    Shotgun
};

struct BulletHole {
    int tileX;
    int tileY;
    GridSegment::Dir dir;   // which face was hit
    float hitFraction;      // normalized along wall
    float lifetime;
    int verticalOffset;
    BulletHoleType type;
};

struct BulletHoleVisual {
    int w = 0;
    int h = 0;
    std::vector<uint32_t> pixels;  // ARGB
};

class BulletHoleManager
{
public:
    bool loadVisual(BulletHoleType type, const std::string& path);
    void update(float dt);
    void spawn(int tileX, int tileY, GridSegment::Dir dir, float hitFraction, BulletHoleType type);

    const std::vector<BulletHole>& getAll() const;
    const BulletHoleVisual& getVisual(BulletHoleType type) const;

private:
    std::unordered_map<BulletHoleType, BulletHoleVisual> visuals;
    std::vector<BulletHole> holes;
    // BulletHoleVisual bulletVisual;

    float maxLifetime = 8.0f;
    size_t maxHoles = 25;
};

