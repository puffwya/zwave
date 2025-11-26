#include "Player.h"
#include "Map.h"
#include <cmath>
#include <SDL2/SDL.h>

void Player::update(float delta, const uint8_t* keys, Map& map) {
    float moveStep = speed * delta;
    float dx = std::cos(angle) * moveStep;
    float dy = std::sin(angle) * moveStep;

    float newX = x;
    float newY = y;

    if (keys[SDL_SCANCODE_UP]) {
        // check axis separately but using floor for conversion to tile coords
        int tx = int(std::floor(x + dx));
        int ty = int(std::floor(y));
        if (map.data[tx][ty] == 0 || map.data[tx][ty] == 2) newX = x + dx;

        tx = int(std::floor(x));
        ty = int(std::floor(y + dy));
        if (map.data[tx][ty] == 0 || map.data[tx][ty] == 2) newY = y + dy;
    } else if (keys[SDL_SCANCODE_DOWN]) {
        int tx = int(std::floor(x - dx));
        int ty = int(std::floor(y));
        if (map.data[tx][ty] == 0 || map.data[tx][ty] == 2) newX = x - dx;

        tx = int(std::floor(x));
        ty = int(std::floor(y - dy));
        if (map.data[tx][ty] == 0 || map.data[tx][ty] == 2) newY = y - dy;
    }

    // apply new pos
    x = newX;
    y = newY;

    if (keys[SDL_SCANCODE_LEFT])  angle -= 2.0f * delta;
    if (keys[SDL_SCANCODE_RIGHT]) angle += 2.0f * delta;

    // item switching debounce
    static bool canSwitchItem = true;

    if (keys[SDL_SCANCODE_Q]) {
        if (canSwitchItem) {
            previousItem();
            canSwitchItem = false;
        }
    } else if (keys[SDL_SCANCODE_E]) {
        if (canSwitchItem) {
            nextItem();
            canSwitchItem = false;
        }
    } else {
        canSwitchItem = true; // reset once no item key is pressed
    }

    // Gets current 10x10 chunk the player is in
    int currentChunk = map.getChunkID(int(std::floor(x)), int(std::floor(y)));
}

void Player::giveItem(ItemType item) {
    inventory.push_back(item);
}

void Player::nextItem() {
    if (inventory.empty()) return;

    auto it = std::find(inventory.begin(), inventory.end(), currentItem);
    
    if (it == inventory.end() || ++it == inventory.end())
        currentItem = inventory[0];
    else
        currentItem = *it;
}

void Player::previousItem() {
    if (inventory.empty()) return;

    auto it = std::find(inventory.begin(), inventory.end(), currentItem);

    if (it == inventory.begin() || it == inventory.end())
        currentItem = inventory.back();
    else
        currentItem = *(--it);
}

