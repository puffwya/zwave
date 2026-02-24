#include "Player.h"
#include "WeaponManager.h"

void Player::renderDamageFlash(uint32_t* pixels, int screenW, int screenH, float intensity)
{
    if (intensity <= 0.0f) return;

    float centerX = screenW * 0.5f;
    float centerY = screenH * 0.5f;
    float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < screenH; y++)
    {
        for (int x = 0; x < screenW; x++)
        {
            float dx = x - centerX;
            float dy = y - centerY;
            float dist = std::sqrt(dx * dx + dy * dy);

            float radial = dist / maxDist;        // 0 center -> 1 edge
            float edgeFactor = radial * radial;   // stronger toward edges

            float alpha = edgeFactor * intensity;

            if (alpha > 0.01f)
            {
                // Blend color over pixel
                uint32_t& pixel = pixels[y * screenW + x];

                uint8_t r = flashR;
                uint8_t g = flashG;
                uint8_t b = flashB;

                // Simple blend
                uint8_t pr = (pixel >> 16) & 0xFF;
                uint8_t pg = (pixel >> 8) & 0xFF;
                uint8_t pb = pixel & 0xFF;

                pr = uint8_t(pr + (r - pr) * alpha);
                pg = uint8_t(pg + (g - pg) * alpha);
                pb = uint8_t(pb + (b - pb) * alpha);

                pixel = (pr << 16) | (pg << 8) | pb;
            }
        }
    }
}

WeaponType Player::itemToWeapon(ItemType item) {
    switch (item) {
        case ItemType::Pistol: return WeaponType::Pistol;
        case ItemType::Shotgun: return WeaponType::Shotgun;
        case ItemType::Mg: return WeaponType::Mg;
        default: return WeaponType::None; // for a safe fallback
    }
}

void Player::applyDamage(int damage, float shieldMultiplier)
{
    if (damage <= 0) return;

    int originalDamage = damage;
    int armorDamage = 0;
    int healthDamage = 0;

    // Armor absorption
    if (armor > 0)
    {
        armorDamage = int(damage * shieldMultiplier);
        armorDamage = std::min(armorDamage, armor);

        armor -= armorDamage;
        damage -= armorDamage;
    }

    // Health damage
    if (damage > 0)
    {
        healthDamage = damage;
        health -= damage;
        if (health < 0) health = 0;
    }

    // Trigger flash if any damage occurred
    if (originalDamage > 0)
    {
        damageFlashTimer = damageFlashDuration;
        damageFlashIntensity = std::min(1.0f, float(originalDamage) / 40.0f);

        if (healthDamage > 0)
        {
            // Health hit = red
            flashR = 255;
            flashG = 0;
            flashB = 0;
        }
        else if (armorDamage > 0)
        {
            // Shield only = blue
            flashR = 40;
            flashG = 120;
            flashB = 255;
        }
    }
}

void Player::update(float delta, const uint8_t* keys, Map& map, EnemyManager& enemyManager, WeaponManager& weaponManager, Weapon& weapon, GameState& gs, AudioManager& audio) {
    // Check if player died
    if (health <= 0) {
        gs = GameState::PlayerDead;
    }

    // Keep track of time elapsed while in game play state
    if (gs == GameState::Playing) {
        timeElapsed += delta;
    }

    // Check for level completion
    if (map.get(int(x), int(y)).isEscape) {
        gs = GameState::LevelEnd;
    }
    // Update damage flash
    if (damageFlashTimer > 0.0f)
    {
        damageFlashTimer -= delta;
        damageFlashIntensity = damageFlashTimer / damageFlashDuration;

        if (damageFlashTimer < 0.0f)
            damageFlashTimer = 0.0f;
    }
    // Lava damage check
    if (onGround)
    {
        if (map.get(int(x), int(y)).isLava)
        {
            lavaTickTimer -= delta;

            if (lavaTickTimer <= 0.0f)
            {
                applyDamage(lavaDamage, 0.25f);
                audio.playSFX("lava_burn");
                lavaTickTimer = lavaTickInterval;
            }
        }
        else
        {
            lavaTickTimer = 0.0f;
        }
    }

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
        velZ = 0.0f;
        z = 0.5f + map.get(int(x), int(y)).height;
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
    if (keys[SDL_SCANCODE_P] && gs != GameState::LevelEnd) {
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

    // Mg shooting
    if (currentItem == ItemType::Mg) {
                
        Uint32 mouseState = SDL_GetMouseState(nullptr, nullptr);
                
        bool leftMouseDown = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
                
        if (leftMouseDown && fireCooldown <= 0.0f) {
            if (weapon.mgClipAmmo <= 0) {
                weapon.mgClipAmmo = 0;
                audio.playSFX("gun_click");
                fireCooldown = 0.7f;
            }
            else {
                shoot(enemyManager, weaponManager, map);
                fireCooldown = 0.1f; // Mg fires once every 0.1 seconds
             
                // Start animation
                isFiringAnim = true;
                fireFrame = 0;  
                fireFrameTimer = 0.01;
                audio.playSFX("mg_shoot");
                weapon.mgClipAmmo -= 1;
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
            else if (currentItem == ItemType::Mg && weapon.mgClipAmmo < weapon.mgClipSize && weapon.mgReserveAmmo != 0) {
                reloading = true;
                reloadFrame = 0;
                reloadFrameTimer = RELOAD_FRAME_DURATION;
                reloadKeyPressed = true;
                weaponManager.playReloadAnimation(itemToWeapon(currentItem));
                audio.playSFX("mg_reload");
            
                // Checks if reserve is less than max clip size and if so sets clip size to reserve
                if (weapon.mgReserveAmmo < weapon.mgClipSize && weapon.mgReserveAmmo > 0) {
                    weapon.mgClipAmmo = weapon.mgReserveAmmo;
                    weapon.mgReserveAmmo = 0;
                }
                // Partial reloads (not on fully empty clip)
                else if (weapon.mgClipAmmo != 0) {
                    weapon.mgReserveAmmo += weapon.mgClipAmmo;
                    weapon.mgClipAmmo = weapon.mgClipSize; 
                    weapon.mgReserveAmmo -= weapon.mgClipSize;
                }
                // "normal case" takes from reserve the max clip size and adds it to clip size
                else {
                    weapon.mgClipAmmo = weapon.mgClipSize;
                    weapon.mgReserveAmmo -= weapon.mgClipSize;
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
            else if (currentItem == ItemType::Mg && fireFrame >= MG_FIRE_FRAMES) {
                // Mg animation over, return to idle
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
            else if (currentItem == ItemType::Mg) {
                RELOAD_FRAMES = 9;   // frames 7–15
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

    // Keep track of shots fired
    shotsFired += 1;

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
            else if (currentItem == ItemType::Mg) {
                damage = 25;
            }
            // Keep track of shots hit
            shotsHit += 1;
            e.takeDamage(damage);

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

