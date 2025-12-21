#include "WeaponManager.h"
#include "Player.h"
#include <cmath>
#include <iostream>

bool WeaponManager::loadAssets(SDL_Renderer* renderer) {

    // load pistol frames
    Animation pistolAnim;

    const char* pistolPaths[] = {
        "Assets/Pistol0.png",
        "Assets/Pistol1.png",
        "Assets/Pistol2.png",
        "Assets/Pistol3.png",
        "Assets/Pistol4.png",
        "Assets/Pistol5.png"
    };

    for (const char* path : pistolPaths) {
        SDL_Texture* tex = IMG_LoadTexture(renderer, path);
        if (!tex) {
            std::cerr << "Failed to load pistol frame: " 
                      << path << " | " << IMG_GetError() << "\n";
            return false;
        }
        pistolAnim.frames.push_back(tex);
    }

    pistolAnim.frameTime = 0.07f; // fast DOOM-style <70ms
    animations[WeaponType::Pistol] = pistolAnim;

    // Load shotgun frames
    Animation shotgunAnim;
    const char* shotgunPaths[] = {
        "Assets/Shotgun0.png",
        "Assets/Shotgun1.png",
        "Assets/Shotgun2.png",
        "Assets/Shotgun3.png",
        "Assets/Shotgun4.png"
    };
    for (const char* path : shotgunPaths) {
        SDL_Texture* tex = IMG_LoadTexture(renderer, path);
        if (!tex) {
            std::cerr << "Failed to load shotgun frame: " << path
                      << " | " << IMG_GetError() << "\n";
            return false;
        }
        shotgunAnim.frames.push_back(tex);
    }
    shotgunAnim.frameTime = 0.1f;
    animations[WeaponType::Shotgun] = shotgunAnim;

    return true;
}

void WeaponManager::startSwap(WeaponType newWeapon) {
    if (swapState != SwapState::Idle)
        return;

    if (newWeapon == currentWeapon)
        return;

    pendingWeapon = newWeapon;

    swapState = SwapState::Lowering;
}

void WeaponManager::playShootAnimation(WeaponType weapon) {
    auto it = animations.find(weapon);
    if (it == animations.end() || it->second.frames.empty()) return; // safe early exit

    Animation& anim = it->second;
    anim.playing = true;
    anim.current = 0;
    anim.timer = 0.0f;
}

int WeaponManager::getDrawOffsetY() const {
    float bob = std::sin(bobTimer) * bobAmount;
    return (int)(swapOffsetY + bob);
}

void WeaponManager::update(float delta, const Player& player) {

    switch (swapState) {
        case SwapState::Idle:
            swapOffsetY = 0.0f;
            break;

        case SwapState::Lowering:
            swapOffsetY += swapSpeed * delta;
            if (swapOffsetY >= maxLowerOffset) {
                swapOffsetY = maxLowerOffset;

                // weapon change
                currentWeapon = pendingWeapon;

                swapState = SwapState::Raising;
            }
            break;

        case SwapState::Raising:
            swapOffsetY -= swapSpeed * delta;
            if (swapOffsetY <= 0.0f) {
                swapOffsetY = 0.0f;
                swapState = SwapState::Idle;
            }
            break;
    }

    for (auto& [weapon, anim] : animations) {
        if (!anim.playing) continue;

        anim.timer += delta;

        if (anim.timer >= anim.frameTime) {
            anim.timer = 0.0f;
            anim.current++;

            if (anim.current >= (int)anim.frames.size()) {
                anim.current = 0;
                anim.playing = false; // stop after 1 cycle
            }
        }
    }

    // determine if player is moving
    bool moving = std::fabs(player.velX) > 0.05f || std::fabs(player.velY) > 0.05f;

    if (moving) {
        bobTimer += delta * 6.0f;   // bobbing speed
        bobAmount = 25.0f;        // pixel magnitude
    } else {
        // reduce amplitude smoothy (frame rate dependent)
        bobAmount = std::lerp(bobAmount, 0.0f, 5.0f * delta);
    }
}

SDL_Texture* WeaponManager::getCurrentFrame(WeaponType weapon) {
    auto it = animations.find(weapon);
    if (it == animations.end() || it->second.frames.empty()) return nullptr; // safe
    return it->second.frames[it->second.current];
}
