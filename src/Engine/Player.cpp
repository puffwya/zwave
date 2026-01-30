#include "Player.h"
#include "WeaponManager.h"

WeaponType Player::itemToWeapon(ItemType item) {
    switch (item) {
        case ItemType::Pistol:  return WeaponType::Pistol;
        case ItemType::Shotgun: return WeaponType::Shotgun;
        default:                return WeaponType::None; // for a safe fallback
    }
}

void Player::applyDamage(int damage, float shieldMultiplier) {
    if (damage <= 0) return;

    if (armor > 0) {
        int shieldDamage = int(damage * shieldMultiplier);

        // Clamp so don't over-drain shield
        shieldDamage = std::min(shieldDamage, armor);

        armor -= shieldDamage;
        damage -= shieldDamage;
    }

    if (damage > 0) {
        health -= damage;
        if (health < 0) health = 0;
    }
}

void Player::update(float delta, const uint8_t* keys, Map& map, EnemyManager& enemyManager, WeaponManager& weaponManager, Weapon& weapon, GameState& gs, AudioManager& audio) {
    // Apply acceleration
    float inputX = 0.0f;
    float inputY = 0.0f;

    footstepTimer -= delta;

    // Move forwards
    if (keys[SDL_SCANCODE_W]) {
        inputX += std::cos(angle);
        inputY += std::sin(angle);
        if (onGround == true && footstepTimer <= 0.0f) {
            audio.playSFX("walk");
            footstepTimer = 0.30f;
        }
    }
    // Move backwards
    if (keys[SDL_SCANCODE_S]) {
        inputX -= std::cos(angle);
        inputY -= std::sin(angle);
        if (onGround == true && footstepTimer <= 0.0f) {
            audio.playSFX("walk");
            footstepTimer = 0.30f;
        }
    }
    // Jump
    if (keys[SDL_SCANCODE_SPACE] && onGround) {
        velZ = JUMP_VELOCITY;
        onGround = false;
        z += 0.001;
        audio.playSFX("jump");
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
    if (!keys[SDL_SCANCODE_W] && !keys[SDL_SCANCODE_S]) {

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

    // Gravity while in the air
    if (onGround == false && z - 0.5 > map.get(int(x), int(y)).height) {

        z += velZ * delta;       // move player vertically
        velZ -= GRAVITY * delta; // apply gravity

        if (z >= map.get(int(x), int(y)).cHeight) {
            z = map.get(int(x), int(y)).cHeight;
            if (velZ > 0) {
                velZ = 0;
            }
        }
    }
    else {
        onGround = true;
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

    // Collision X-axis
    int tx = int(std::floor(newX));
    int ty = int(std::floor(y));
    auto& tileX = map.get(tx, ty);

    float heightDelta = tileX.height - (z - 0.5);

    if (heightDelta <= 0.25f) {
        x = newX;

        // Update player z to 0.5 greater than tile height
        if (z != 0.5 + tileX.height && onGround) {
            onGround = false;

            // Gradually move z toward targetZ
            float baseFallSpeed = 2.0f;   // minimum speed for tiny steps
            float maxFallSpeed  = 6.0f;   // maximum speed for tall drops

            // Use the tile height as a scaling factor
            float heightFactor = std::abs(tileX.height); // use absolute for negative pits
            float fallSpeed = baseFallSpeed + heightFactor * 4.0f;

            // Clamp to prevent extreme speed
            fallSpeed = std::clamp(fallSpeed, baseFallSpeed, maxFallSpeed);

            if (z < 0.5f + tileX.height) {
                z += fallSpeed * delta;
                if (z > 0.5f + tileX.height) z = 0.5f + tileX.height; // clamp so we don't overshoot
            } else if (z > 0.5f + tileX.height) {
                z -= fallSpeed * delta;
                if (z < 0.5f + tileX.height) z = 0.5f + tileX.height;
            }
        }
    } else {
        velX = 0;
    }

    // Collision Y-axis
    tx = int(std::floor(x));
    ty = int(std::floor(newY));
    auto& tileY = map.get(tx, ty);

    heightDelta = tileY.height - (z - 0.5);

    if (heightDelta <= 0.25f) {
        y = newY;

        // Update player z to 0.5 greater than tile height
        if (z != 0.5 + tileY.height && onGround) {
            onGround = false;
    
            // Gradually move z toward targetZ
            float baseFallSpeed = 2.0f;   // minimum speed for tiny steps
            float maxFallSpeed  = 6.0f;   // maximum speed for tall drops
            
            // Use the tile height as a scaling factor
            float heightFactor = std::abs(tileY.height); // use absolute for negative pits
            float fallSpeed = baseFallSpeed + heightFactor * 4.0f;                             
            
            // Clamp to prevent extreme speed
            fallSpeed = std::clamp(fallSpeed, baseFallSpeed, maxFallSpeed);

            if (z < 0.5f + tileY.height) {
                z += fallSpeed * delta;
                if (z > 0.5f + tileY.height) z = 0.5f + tileY.height; // clamp so we don't overshoot
            } else if (z > 0.5f + tileY.height) {
                z -= fallSpeed * delta;
                if (z < 0.5f + tileY.height) z = 0.5f + tileY.height;
            }
        }
    } else {
        velY = 0;
    }

    // Turning acceleration
    if (keys[SDL_SCANCODE_A]) {
        turnVel -= TURN_ACCEL * delta;
    }
    else if (keys[SDL_SCANCODE_D]) {
        turnVel += TURN_ACCEL * delta;
    }
    else {
        // Apply friction when no turning keys are pressed
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
            audio.playSFX("item_swap");
        }
    } else if (keys[SDL_SCANCODE_E]) {
        if (canSwitchItem) {
            ItemType n = nextItem();
            weaponManager.startSwap(itemToWeapon(n));
            currentItem = n;
            canSwitchItem = false;
            audio.playSFX("item_swap");
        }
    } else {
        canSwitchItem = true; // reset once no item key is pressed
    }

    // Handle shooting
    
    // Pistol shooting
    if (currentItem == ItemType::Pistol) {

        Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
        
        bool leftMouseDown = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);

        if (leftMouseDown && fireCooldown <= 0.0f) {
            if (weapon.pClipAmmo <= 0) {
                weapon.pClipAmmo = 0;
                audio.playSFX("gun_click");
                fireCooldown = 0.75f;
            }
            else {
                shoot(enemyManager, weaponManager, map);
                fireCooldown = 0.75f; // pistol fires once every 0.75 seconds

                // Start animation
                isFiringAnim = true;
                fireFrame = 0;
                fireFrameTimer = FIRE_FRAME_DURATION;
                audio.playSFX("pistol_shoot", 0.6f);
                weapon.pClipAmmo -= 1;
            }
        }
    }

    // Shotgun shooting
    if (currentItem == ItemType::Shotgun) {

        Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);

        bool leftMouseDown = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);

        if (leftMouseDown && fireCooldown <= 0.0f) {
            if (weapon.sClipAmmo <= 0) {
                weapon.sClipAmmo = 0;
                audio.playSFX("gun_click");
                fireCooldown = 0.75f;
            }
            else {
                shoot(enemyManager, weaponManager, map);
                fireCooldown = 0.75f; // Shotgun fires once every 0.75 seconds

                // Start animation
                isFiringAnim = true;
                fireFrame = 0;
                fireFrameTimer = FIRE_FRAME_DURATION;
                audio.playSFX("shotgun_shoot");
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
                audio.playSFX("shotgun_reload");

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
                audio.playSFX("pistol_reload");
                
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
    const float hitWidth = 0.08f;
    const float maxRange = 10.0f;

    WeaponType wt = itemToWeapon(currentItem);

    if (wt != WeaponType::None)
        weaponManager.playShootAnimation(wt);

    float dirX = std::cos(angle);
    float dirY = std::sin(angle);

    const float fovScale = 0.66f;
    float planeX = -dirY * fovScale;
    float planeY =  dirX * fovScale;

    for (int i = 0; i < manager.MAX_ENEMIES; i++) {
        Enemy& e = manager.enemies[i];
        if (!e.active) continue;

        float dx = e.x - x;
        float dy = e.y - y;
        float dist = std::sqrt(dx*dx + dy*dy);
        if (dist > maxRange) continue;

        float invDet = 1.0f / (planeX * dirY - dirX * planeY);

        float transformX = invDet * (dirY * dx - dirX * dy);
        float transformY = invDet * (-planeY * dx + planeX * dy);

        if (transformY <= 0) continue;

        float lateral = transformX / transformY;

        if (std::fabs(lateral) < hitWidth) {

            if (!e.hasLineOfSight(*this, map)) {
                continue;
            }

            // Apply damage
            int damage = 0;
            if (currentItem == ItemType::Pistol) {
                damage = 50;
            }
            else if (currentItem == ItemType::Shotgun) {
                damage = 200;
            }
            e.takeDamage(damage);

            if (e.isDead()) {
                e.deactivate();
                printf("Enemy killed!\n");
            } else {
                printf("Enemy hit! HP: %d\n", e.health);
            }

            break;
        }
    }

    isFiringAnim = true;
    fireFrame = 1;
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

