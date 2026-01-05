#include "Player.h"
#include "WeaponManager.h"

WeaponType Player::itemToWeapon(ItemType item) {
    switch (item) {
        case ItemType::Pistol:  return WeaponType::Pistol;
        case ItemType::Shotgun: return WeaponType::Shotgun;
        default:                return WeaponType::None; // for a safe fallback
    }
}

void Player::update(float delta, const uint8_t* keys, Map& map, EnemyManager& enemyManager, WeaponManager& weaponManager, Weapon& weapon, GameState& gs) {
    // Apply acceleration
    float inputX = 0.0f;
    float inputY = 0.0f;

    if (keys[SDL_SCANCODE_UP]) {
        inputX += std::cos(angle);
        inputY += std::sin(angle);
    }
    if (keys[SDL_SCANCODE_DOWN]) {
        inputX -= std::cos(angle);
        inputY -= std::sin(angle);
    }

    // Normalize input so diagonal isn’t faster
    float len = std::sqrt(inputX*inputX + inputY*inputY);
    if (len > 0.01f) {
        inputX /= len;
        inputY /= len;

        velX += inputX * ACCEL * delta;
        velY += inputY * ACCEL * delta;
    }

    // Apply friction if no movement keys
    if (!keys[SDL_SCANCODE_UP] && !keys[SDL_SCANCODE_DOWN]) {

        float speed = std::sqrt(velX*velX + velY*velY);

        if (speed > 0.0001f) {
            float drop = FRICTION * delta;
            float newSpeed = speed - drop;
            if (newSpeed < 0) newSpeed = 0;

            float ratio = newSpeed / speed;
            velX *= ratio;
            velY *= ratio;
        }
    }

    // Clamp max speed
    float speedNow = std::sqrt(velX*velX + velY*velY);
    if (speedNow > MAX_SPEED) {
        velX = (velX / speedNow) * MAX_SPEED;
        velY = (velY / speedNow) * MAX_SPEED;
    }

    // Proposed new position
    float newX = x + velX * delta;
    float newY = y + velY * delta;

    // --- Collision X-axis ---
    int tx = int(std::floor(newX));
    int ty = int(std::floor(y));
    auto& tileX = map.get(tx, ty);

    if (tileX.type != Map::TileType::Wall || tileX.height < 0.0f) {
        x = newX;

        // --- Floor / Pit logic ---
        float floorZ = tileX.height;
        if (z > floorZ && floorZ != 0.0f) {
            z = 0.3;
        }
        else if (floorZ == 0.0f) {
            z = 0.5f;
        }
    } else {
        velX = 0;
    }

    // --- Collision Y-axis ---
    tx = int(std::floor(x));
    ty = int(std::floor(newY));
    auto& tileY = map.get(tx, ty);
    
    if (tileY.type != Map::TileType::Wall || tileY.height < 0.0f) {
        y = newY;

        float floorZ = tileY.height;
        if (z > floorZ && floorZ != 0.0f) {
            z = 0.3;
        }
        else if (floorZ == 0.0f) {
            z = 0.5f;
        }
    } else {
        velY = 0;
    }

    // Turning acceleration
    if (keys[SDL_SCANCODE_LEFT]) {
        turnVel -= TURN_ACCEL * delta;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        turnVel += TURN_ACCEL * delta;
    }
    else {
        // apply friction when NO turning keys are pressed
        if (fabs(turnVel) > 0.0001f) {
            float drop = TURN_FRICTION * delta;
            if (turnVel > 0) {
                turnVel = std::max(0.0f, turnVel - drop);
            } else {
                turnVel = std::min(0.0f, turnVel + drop);
            }
        }
    }

    // clamp turning speed
    if (turnVel >  MAX_TURN_SPEED) turnVel =  MAX_TURN_SPEED;
    if (turnVel < -MAX_TURN_SPEED) turnVel = -MAX_TURN_SPEED;

    // update angle
    angle += turnVel * delta;

    // wrap angle
    if (angle < 0) angle += 2*M_PI;
    if (angle >= 2*M_PI) angle -= 2*M_PI;

    // Pause Handling
    if (keys[SDL_SCANCODE_P]) {
        gs = GameState::Paused;
    }

    // item switching debounce
    static bool canSwitchItem = true;

    if (keys[SDL_SCANCODE_Q]) {
        if (canSwitchItem) {
            ItemType p = previousItem();
            weaponManager.startSwap(itemToWeapon(p));
            currentItem = p;
            canSwitchItem = false;
        }
    } else if (keys[SDL_SCANCODE_E]) {
        if (canSwitchItem) {
            ItemType n = nextItem();
            weaponManager.startSwap(itemToWeapon(n));
            currentItem = n;
            canSwitchItem = false;
        }
    } else {
        canSwitchItem = true; // reset once no item key is pressed
    }

    // Handle shooting
    
    // Pistol shooting
    if (currentItem == ItemType::Pistol) {
        if (keys[SDL_SCANCODE_SPACE] && fireCooldown <= 0.0f) {
            if (weapon.pClipAmmo <= 0) {
                // Will play clicking sound in future
                weapon.pClipAmmo = 0;
            }
            else {
                shoot(enemyManager, weaponManager, map);
                fireCooldown = 0.5f; // pistol fires once every 0.5 seconds

                // Start animation
                isFiringAnim = true;
                fireFrame = 0;
                fireFrameTimer = FIRE_FRAME_DURATION;
                weapon.pClipAmmo -= 1;
            }
        }
    }

    // Shotgun shooting
    if (currentItem == ItemType::Shotgun) {
        if (keys[SDL_SCANCODE_SPACE] && fireCooldown <= 0.0f) {
            if (weapon.sClipAmmo <= 0) {
                // Will play clicking sound in future  
                //weapon.sClipAmmo = 0;
            }
            else {
                shoot(enemyManager, weaponManager, map);
                fireCooldown = 0.5f; // Shotgun fires once every 0.5 seconds

                // Start animation
                isFiringAnim = true;
                fireFrame = 0;
                fireFrameTimer = FIRE_FRAME_DURATION;
                weapon.sClipAmmo -= 2;
            }
        }
    }

    // Reload guns
    if (keys[SDL_SCANCODE_R]) {
        if (!reloadKeyPressed) {
            if (currentItem == ItemType::Shotgun && weapon.sClipAmmo < weapon.sClipSize && weapon.sReserveAmmo != 0) {
                reloading = true;
                reloadFrame = 0;
                reloadFrameTimer = RELOAD_FRAME_DURATION;
                reloadKeyPressed = true;
                weaponManager.playReloadAnimation(itemToWeapon(currentItem));

                // Checks if reserve is less than max clip size and if so sets clip size to reserve
                if (weapon.sReserveAmmo < weapon.sClipSize && weapon.sReserveAmmo > 0) {
                    weapon.sClipAmmo = weapon.sReserveAmmo;
                    weapon.sReserveAmmo = 0;
                }
                // "normal case" takes from reserve the max clip size and adds it to clip size
                else {
                    weapon.sClipAmmo = weapon.sClipSize;
                    weapon.sReserveAmmo -= weapon.sClipSize;
                }
            }
            else if (currentItem == ItemType::Pistol && weapon.pClipAmmo < weapon.pClipSize && weapon.pReserveAmmo != 0) {
                reloading = true;
                reloadFrame = 0;
                reloadFrameTimer = RELOAD_FRAME_DURATION;
                reloadKeyPressed = true;
                weaponManager.playReloadAnimation(itemToWeapon(currentItem));
                
                // Checks if reserve is less than max clip size and if so sets clip size to reserve
                if (weapon.pReserveAmmo < weapon.pClipSize && weapon.pReserveAmmo > 0) {
                    weapon.pClipAmmo = weapon.pReserveAmmo;
                    weapon.pReserveAmmo = 0;
                }
                // Partial reloads (not on fully empty clip)
                else if (weapon.pClipAmmo != 0) {
                    weapon.pReserveAmmo += weapon.pClipAmmo;
                    weapon.pClipAmmo = weapon.pClipSize;
                    weapon.pReserveAmmo -= weapon.pClipSize;
                }
                // "normal case" takes from reserve the max clip size and adds it to clip size
                else {
                    weapon.pClipAmmo = weapon.pClipSize;
                    weapon.pReserveAmmo -= weapon.pClipSize;
                }
            }
        }
    } else {
        reloadKeyPressed = false;
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

    if (reloading) {
        reloadFrameTimer -= delta;
        if (reloadFrameTimer <= 0.0f) {
            reloadFrame++;
            reloadFrameTimer = RELOAD_FRAME_DURATION;

            int RELOAD_FRAMES = 0;

            if (currentItem == ItemType::Shotgun) {
                RELOAD_FRAMES = 7;   // frames 4–10
            }
            else if (currentItem == ItemType::Pistol) {
                RELOAD_FRAMES = 7;   // frames 6–12
            }

            if (reloadFrame >= RELOAD_FRAMES) {
                // Reload finished
                reloadFrame = 0;
                reloading = false; 
            }
        }
    }


    // Gets current 10x10 chunk the player is in
    int currentChunk = map.getChunkID(int(std::floor(x)), int(std::floor(y)));
}

void Player::shoot(EnemyManager& manager, WeaponManager& weaponManager, Map& map) {
    const float hitWidth = 0.08f;   // how wide the hit cone is
    const float maxRange = 10.0f;  // max distance pistol can hit

    WeaponType wt = itemToWeapon(currentItem);

    // play animation
    if (wt != WeaponType::None)
        weaponManager.playShootAnimation(wt);

    // convert angle into direction vector
    float dirX = std::cos(angle);
    float dirY = std::sin(angle);

    // camera plane (controls FOV)
    const float fovScale = 0.66f;  // 66° FOV like DOOM
    float planeX = -dirY * fovScale;
    float planeY =  dirX * fovScale;


    for (int i = 0; i < manager.MAX_ENEMIES; i++) {
        Enemy& e = manager.enemies[i];
        if (!e.active) continue;

        float dx = e.x - x;
        float dy = e.y - y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > maxRange) continue;  // out of range

        // camera space transform
        float invDet = 1.0f / (planeX * dirY - dirX * planeY);

        float transformX = invDet * (dirY * dx - dirX * dy);
        float transformY = invDet * (-planeY * dx + planeX * dy);

        // If enemy is behind player
        if (transformY <= 0) continue;

        // Hit if enemy is near the center of the screen (= small X offset)
        float lateral = transformX / transformY;

        if (std::fabs(lateral) < hitWidth) {

            // checks if enemy is behind a wall
            if (!e.hasLineOfSight(*this, map)) {
                continue;  // if enemy is behind a wall -> cannot be hit
            }

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

// Returns next item
ItemType Player::nextItem() const {
    if (inventory.empty())
        return currentItem;

    auto it = std::find(inventory.begin(), inventory.end(), currentItem);

    if (it == inventory.end() || ++it == inventory.end())
        return inventory[0];

    return *it;
}

// Returns prev item
ItemType Player::previousItem() const {
    if (inventory.empty())
        return currentItem;

    auto it = std::find(inventory.begin(), inventory.end(), currentItem);

    if (it == inventory.begin() || it == inventory.end())
        return inventory.back();

    return *(--it);
}

