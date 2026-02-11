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
        "Assets/Pistol5.png",
        "Assets/Pistol6.png",
        "Assets/Pistol7.png",
        "Assets/Pistol8.png",
        "Assets/Pistol9.png",
        "Assets/Pistol10.png",
        "Assets/Pistol11.png",
        "Assets/Pistol12.png"
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
    pistolAnim.frameTime = 0.09f; // <90ms
    animations[WeaponType::Pistol] = pistolAnim;

    // Load shotgun frames
    Animation shotgunAnim;
    const char* shotgunPaths[] = {
        "Assets/Shotgun0.png",
        "Assets/Shotgun1.png",
        "Assets/Shotgun2.png",
        "Assets/Shotgun3.png",
        "Assets/Shotgun4.png",
        "Assets/Shotgun5.png",
        "Assets/Shotgun6.png",
        "Assets/Shotgun7.png",
        "Assets/Shotgun8.png",
        "Assets/Shotgun9.png",
        "Assets/Shotgun10.png"
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
    shotgunAnim.frameTime = 0.14f;
    animations[WeaponType::Shotgun] = shotgunAnim;

    // Load MG frames
    Animation mgAnim;
    const char* mgPaths[] = {
        "Assets/Mg0.png",
        "Assets/Mg1.png",
        "Assets/Mg2.png",
        "Assets/Mg3.png",
        "Assets/Mg4.png",
        "Assets/Mg5.png",
        "Assets/Mg6.png",
        "Assets/Mg7.png",
        "Assets/Mg8.png",
        "Assets/Mg9.png",
        "Assets/Mg10.png",
        "Assets/Mg11.png",
        "Assets/Mg12.png",
        "Assets/Mg13.png",
        "Assets/Mg14.png",
        "Assets/Mg15.png",
    };
    for (const char* path : mgPaths) {
        SDL_Texture* tex = IMG_LoadTexture(renderer, path);
        if (!tex) {
            std::cerr << "Failed to load mg frame: " << path
                      << " | " << IMG_GetError() << "\n";  
            return false;
        }
        mgAnim.frames.push_back(tex);
    }
    mgAnim.frameTime = 0.01f;
    animations[WeaponType::Mg] = mgAnim;

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

void WeaponManager::playReloadAnimation(WeaponType weapon)
{
    auto it = animations.find(weapon);
    if (it == animations.end() || it->second.frames.empty()) return;

    Animation& anim = it->second;
    
    // Set reload frame range
    if (weapon == WeaponType::Shotgun) {
        anim.startFrame = 4;
        anim.endFrame = 11; // exclusive, frames 4-10
    }
    // Set reload frame range
    else if (weapon == WeaponType::Pistol) {
        anim.startFrame = 6;
        anim.endFrame = 13;
    }
    // Set reload frame range
    else if (weapon == WeaponType::Mg) {
        anim.startFrame = 7;
        anim.endFrame = 16;
        anim.frameTime = 0.05f;
    }
    
    anim.current = anim.startFrame;  // only set once here
    anim.timer = 0.0f;
    anim.playing = true;
}

void WeaponManager::playShootAnimation(WeaponType weapon) {
    auto it = animations.find(weapon);
    if (it == animations.end()) return;

    Animation& anim = it->second;

    if (weapon == WeaponType::Mg) {
        anim.frameTime = 0.02;
    }

    if (!anim.playing) {
        anim.playing = true;
        anim.timer = 0.0f;
        anim.current = 0;
    }
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

        while (anim.timer >= anim.frameTime) {
            anim.timer -= anim.frameTime;
            anim.current++;

            int endFrame = anim.frames.size();

            if (!player.reloading) {
                if (currentWeapon == WeaponType::Pistol) endFrame = 6;
                else if (currentWeapon == WeaponType::Shotgun) endFrame = 4;
                else if (currentWeapon == WeaponType::Mg) endFrame = 7;
            }

            if (anim.current >= endFrame) {
                anim.current = 0;
                anim.playing = false;
                break;
            }
        }
    }

    // determine if player is moving
    bool moving = std::fabs(player.velX) > 0.05f || std::fabs(player.velY) > 0.05f;

    if (moving) {
        bobTimer += delta * 6.0f; // bobbing speed
        bobAmount = 25.0f; // pixel magnitude
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
