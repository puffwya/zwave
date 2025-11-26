#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include "Map.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

enum class ItemType {
    None,
    Pistol,
    Shotgun,
    // Add more weapons here
};


class Player {
public:
    float x, y;
    float angle;
    float speed;

    ItemType currentItem = ItemType::None;
    std::vector<ItemType> inventory;

    int lastChunkID;

    Player(float startX = 3.5f, float startY = 3.5f, float startAngle = 0.0f)
        : x(startX), y(startY), angle(startAngle), speed(3.0f) {
        lastChunkID = -1;
    }

    void update(float delta, const uint8_t* keys, Map& map);
    void giveItem(ItemType item);
    void nextItem();
    void previousItem();

};

#endif

