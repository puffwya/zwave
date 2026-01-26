#pragma once
#include "PickupType.h"
#include "Player.h"

struct Pickup {
    float x, y, z = 0.0f;
    float height = 0.5f;
    PickupType type;
    bool collected = false;

    Pickup(float x_, float y_, PickupType t)
        : x(x_), y(y_), type(t) {}

    void apply(Player& player);
};

