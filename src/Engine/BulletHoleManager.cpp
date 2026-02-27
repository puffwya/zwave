#include "BulletHoleManager.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

bool BulletHoleManager::loadVisual(
    BulletHoleType type,
    const std::string& texturePath
) {
    SDL_Surface* surf = IMG_Load(texturePath.c_str());
    if (!surf) {
        std::cerr << "Failed to load bullet hole texture: "
                  << texturePath << "\n";
        return false;
    }

    SDL_Surface* formatted =
        SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);
    if (!formatted) return false;

    BulletHoleVisual visual;
    visual.w = formatted->w;
    visual.h = formatted->h;
    visual.pixels.resize(visual.w * visual.h);

    std::memcpy(
        visual.pixels.data(),
        formatted->pixels,
        visual.w * visual.h * 4
    );

    SDL_FreeSurface(formatted);

    visuals[type] = std::move(visual);
    return true;
}

void BulletHoleManager::update(float dt) {
    for (auto it = holes.begin(); it != holes.end(); ) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.0f)
            it = holes.erase(it);
        else
            ++it;
    }
}

void BulletHoleManager::spawn(
    int tileX,
    int tileY,
    GridSegment::Dir dir,
    float hitFraction,
    BulletHoleType type
) {
    BulletHole hole;
    hole.tileX = tileX;
    hole.tileY = tileY;
    hole.dir = dir;
    hole.hitFraction = hitFraction;
    hole.lifetime = maxLifetime;
    hole.verticalOffset = (std::rand() % 5) - 2;
    hole.type = type;

    holes.push_back(hole);
    if (holes.size() > maxHoles)
        holes.erase(holes.begin());
}

const std::vector<BulletHole>& BulletHoleManager::getAll() const { return holes; }
const BulletHoleVisual& BulletHoleManager::getVisual(BulletHoleType type) const { return visuals.at(type); }

