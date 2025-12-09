#include "Player.h"
#include "WeaponManager.h"

WeaponType Player::itemToWeapon(ItemType item) {
    switch (item) {
        case ItemType::Pistol:  return WeaponType::Pistol;
        case ItemType::Shotgun: return WeaponType::Shotgun;
        default:                return WeaponType::None; // for a safe fallback
    }
}

void Player::update(float delta, const uint8_t* keys, Map& map, EnemyManager& enemyManager, WeaponManager& weaponManager) {
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

    // Collision
    int tx = int(std::floor(newX));
    int ty = int(std::floor(y));
    if (map.get(tx, ty).type != Map::TileType::Wall) {
        x = newX;
    } else {
        velX = 0;
    }

    tx = int(std::floor(x));
    ty = int(std::floor(newY));
    if (map.get(tx, ty).type != Map::TileType::Wall) {
        y = newY;
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
            shoot(enemyManager, weaponManager, map);
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
            shoot(enemyManager, weaponManager, map);
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

