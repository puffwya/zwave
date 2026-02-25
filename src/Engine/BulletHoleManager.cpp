#include "BulletHoleManager.h"
#include <SDL2/SDL_image.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>

bool BulletHoleManager::init(const std::string& texturePath) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    SDL_Surface* surf = IMG_Load(texturePath.c_str());
    if (!surf) {
        std::cerr << "Failed to load bullet hole texture: " << texturePath << "\n";
        return false;
    }

    SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);
    if (!formatted) return false;

    bulletVisual.w = formatted->w;
    bulletVisual.h = formatted->h;
    bulletVisual.pixels.resize(bulletVisual.w * bulletVisual.h);
    std::memcpy(bulletVisual.pixels.data(), formatted->pixels, bulletVisual.w * bulletVisual.h * 4);
    SDL_FreeSurface(formatted);

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

void BulletHoleManager::spawn(int tileX, int tileY, GridSegment::Dir dir, float hitFraction) {
    BulletHole hole;
    hole.tileX = tileX;
    hole.tileY = tileY;
    hole.dir = dir;
    hole.hitFraction = hitFraction;
    hole.lifetime = maxLifetime;
    hole.verticalOffset = (std::rand() % 5) - 2;
    holes.push_back(hole);

    if (holes.size() > maxHoles)
        holes.erase(holes.begin());
}

const std::vector<BulletHole>& BulletHoleManager::getAll() const { return holes; }
const BulletHoleVisual& BulletHoleManager::getVisual() const { return bulletVisual; }

