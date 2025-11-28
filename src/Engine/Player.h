#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include "Map.h"
#include "EnemyManager.h"
#include "WeaponManager.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

enum class ItemType {
    None,
    Pistol,
    Shotgun,
    // future weapons to be added
};


class Player {
public:
    float x, y;
    float angle;
    float speed;

    ItemType currentItem = ItemType::None;
    std::vector<ItemType> inventory;

    // Shooting
    bool isShooting = false;      // true if player is shooting
    float fireCooldown = 0.0f;    // timer to control fire rate

    // Shooting Anim
    bool isFiringAnim = false;
    int fireFrame = 0;
    float fireFrameTimer = 0.0f;

    static constexpr float FIRE_FRAME_DURATION = 0.05f; // 50ms per frame
    static constexpr int PISTOL_FIRE_FRAMES = 6;        // idle + 5 Pistol firing frames
    static constexpr int SHOTGUN_FIRE_FRAMES = 5;        // idle + 4 Shotgun firing frames


    int lastChunkID;

    Player(float startX = 3.5f, float startY = 3.5f, float startAngle = 0.0f)
        : x(startX), y(startY), angle(startAngle), speed(3.0f) {
        lastChunkID = -1;
    }

    void update(float delta, const uint8_t* keys, Map& map, EnemyManager& enemyManager, WeaponManager& weaponManager);
    void shoot(EnemyManager& manager, WeaponManager& weaponManager, Map& map);

    void giveItem(ItemType item);
    void nextItem();
    void previousItem();

    WeaponType itemToWeapon(ItemType item);

};

#endif

