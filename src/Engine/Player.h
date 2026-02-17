#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include "Map.h"
#include "EnemyManager.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>
#include "WeaponTypes.h"
#include "GameState.h"
#include "../audio/AudioManager.h"

class WeaponManager;

enum class ItemType {
    None,
    Pistol,
    Shotgun,
    Mg
};


class Player {
public:
    int health = 50;
    int maxHealth = 100;
    int armor = 25;
    int maxArmor = 100;

    float x, y, z;
    float angle;
    float speed;

    float velX = 0.0f;
    float velY = 0.0f;
    float velZ = 0.0f;

    float damageFlashTimer = 0.0f;
    float damageFlashDuration = 0.35f;   // how long flash lasts
    float damageFlashIntensity = 0.0f;

    float lavaTickTimer = 0.0f;
    float lavaTickInterval = 0.5f;   // damage every 0.5 seconds
    int lavaDamage = 5;              // damage per tick

    bool onGround = true;

    const float ACCEL = 14.0f;       // how fast player reaches max speed
    const float FRICTION = 8.0f;    // how fast player slides to a stop
    const float MAX_SPEED = 6.0f;   // top movement speed

    float turnVel = 0.0f;
    const float TURN_ACCEL = 10.0f;    // how fast turning speeds up
    const float TURN_FRICTION = 14.0f; // how fast turning slows down
    const float MAX_TURN_SPEED = 2.5f; // top turning speed

    const float JUMP_VELOCITY = 3.0f;
    const float GRAVITY = 9.8f;

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
    static constexpr int PISTOL_FIRE_FRAMES = 6; // idle + 5 Pistol firing frames
    static constexpr int SHOTGUN_FIRE_FRAMES = 4; // idle + 3 Shotgun firing frames
    static constexpr int MG_FIRE_FRAMES = 7; // idle + 6 Mg firing frames

    // Reload Anim 
    bool reloading = false;
    bool reloadKeyPressed = false;
    int reloadFrame = 0;
    float reloadFrameTimer = 0.0f;
    const float RELOAD_FRAME_DURATION = 0.1f;

    // Footstep sfx
    float footstepTimer = 0.0f;

    int lastChunkID;

    Player(float startX = 14.5f, float startY = 2.5f, float startZ = 0.5f, float startAngle = 0.0f)
        : x(startX), y(startY), angle(startAngle), speed(3.0f), z(startZ) {
        lastChunkID = -1;
    }

    void update(float delta, const uint8_t* keys, Map& map, EnemyManager& enemyManager, WeaponManager& weaponManager, Weapon& weapon, GameState& gs, AudioManager& audio);
    void shoot(EnemyManager& manager, WeaponManager& weaponManager, Map& map);

    void giveItem(ItemType item);
    ItemType nextItem() const;
    ItemType previousItem() const;

    WeaponType itemToWeapon(ItemType item);

    void applyDamage(int damage, float shieldMultiplier);

    void renderDamageFlash(uint32_t* pixels, int screenW, int screenH, float intensity);

    uint8_t flashR = 255;
    uint8_t flashG = 0;
    uint8_t flashB = 0;
};

#endif

