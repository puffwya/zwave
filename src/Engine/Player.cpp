#include "Player.h"
#include "Map.h"
#include <cmath>
#include <SDL2/SDL.h>

WeaponType Player::itemToWeapon(ItemType item) {
    switch (item) {
        case ItemType::Pistol:  return WeaponType::Pistol;
        case ItemType::Shotgun: return WeaponType::Shotgun;
        default:                return WeaponType::None; // for a safe fallback
    }
}

void Player::update(float delta, const uint8_t* keys, Map& map, EnemyManager& enemyManager, WeaponManager& weaponManager) {
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

    // Handle shooting
    
    // Pistol shooting
    if (currentItem == ItemType::Pistol) {
        if (keys[SDL_SCANCODE_SPACE] && fireCooldown <= 0.0f) {
            shoot(enemyManager, weaponManager);
            fireCooldown = 0.5f; // pistol fires once every 0.5 seconds

            // Start animation
            isFiringAnim = true;
            fireFrame = 0;
            fireFrameTimer = FIRE_FRAME_DURATION;
        }
    }

    // Shotgun shooting
    if (currentItem == ItemType::Shotgun) {
        if (keys[SDL_SCANCODE_SPACE] && fireCooldown <= 0.0f) {
            shoot(enemyManager, weaponManager);
            fireCooldown = 0.5f; // Shotgun fires once every 0.5 seconds

            // Start animation
            isFiringAnim = true;
            fireFrame = 0;
            fireFrameTimer = FIRE_FRAME_DURATION;
        }
    }

    // Reduce cooldown
    if (fireCooldown > 0.0f)
        fireCooldown -= delta;

    // Animation update
    if (isFiringAnim) {
        fireFrameTimer -= delta;
        if (fireFrameTimer <= 0.0f) {
            fireFrame++;
            fireFrameTimer = FIRE_FRAME_DURATION;

            if (currentItem == ItemType::Pistol && fireFrame >= PISTOL_FIRE_FRAMES) {
                // Pistol animation over, return to idle
                fireFrame = 0;
                isFiringAnim = false;
            }
            else if (currentItem == ItemType::Shotgun && fireFrame >= SHOTGUN_FIRE_FRAMES) {
                // Shotgun animation over, return to idle
                fireFrame = 0;
                isFiringAnim = false;
            }
        }
    }


    // Gets current 10x10 chunk the player is in
    int currentChunk = map.getChunkID(int(std::floor(x)), int(std::floor(y)));
}

void Player::shoot(EnemyManager& manager, WeaponManager& weaponManager) {
    const float maxAngle = 0.1f;   // tolerance in radians (~5-6 degrees)
    const float maxRange = 10.0f;  // max distance pistol can hit

    WeaponType wt = itemToWeapon(currentItem);

    // play animation
    if (wt != WeaponType::None)
        weaponManager.playShootAnimation(wt);

    for (int i = 0; i < manager.MAX_ENEMIES; ++i) {
        Enemy& e = manager.enemies[i];
        if (!e.active) continue;

        float dx = e.x - x;
        float dy = e.y - y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > maxRange) continue;  // out of range

        float angleToEnemy = std::atan2(dy, dx);
        float diff = angleToEnemy - angle;
        if (diff < -M_PI) diff += 2*M_PI;
        if (diff >  M_PI) diff -= 2*M_PI;

        if (std::fabs(diff) < maxAngle) {
            e.active = false; // enemy hit
            printf("Enemy hit!\n");
            break; // stop after hitting first enemy
        }
    }
    isFiringAnim = true;
    fireFrame = 1;          // Start on first firing frame
    fireFrameTimer = FIRE_FRAME_DURATION;
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

