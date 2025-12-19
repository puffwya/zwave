#pragma once
#include <cstdint>
#include "Player.h"
#include "Map.h"

// ------------------------------------
// OLD RAYCASTER ENGINE NO LONGER IN USE
// ------------------------------------

class Raycaster {
public:
    // render fills pixels[] and writes perpendicular wall distances into zBuffer (length = screenW)
    void render(uint32_t* pixels, int screenW, int screenH,
                const Player& player, const Map& map, float* zBuffer);
};

