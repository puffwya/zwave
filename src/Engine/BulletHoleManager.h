#pragma once
#include <vector>
#include <string>
#include "MapToSegments.h"

struct BulletHole {
    int tileX;
    int tileY;
    GridSegment::Dir dir;   // which face was hit
    float hitFraction;      // normalized along wall
    float lifetime;
    int verticalOffset;
};

struct BulletHoleVisual {
    int w = 0;
    int h = 0;
    std::vector<uint32_t> pixels;  // ARGB
};

class BulletHoleManager
{
public:
    bool init(const std::string& texturePath);
    void update(float dt);
    void spawn(int tileX, int tileY, GridSegment::Dir dir, float hitFraction);

    const std::vector<BulletHole>& getAll() const;
    const BulletHoleVisual& getVisual() const;

private:
    std::vector<BulletHole> holes;
    BulletHoleVisual bulletVisual;

    float maxLifetime = 8.0f;
    size_t maxHoles = 25;
};

