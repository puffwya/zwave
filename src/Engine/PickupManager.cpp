#include "PickupManager.h"
#include "Player.h"
#include "Map.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <SDL2/SDL_image.h>

// Helper to load a PNG into a PickupVisual
bool PickupManager::loadPickupFrame(const std::string& path, PickupVisual& out) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cerr << "Failed to load pickup: " << path << "\n";
        return false;
    }

    SDL_Surface* formatted = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);
    if (!formatted) return false;

    out.w = formatted->w;
    out.h = formatted->h;
    out.pixels.resize(out.w * out.h);
    std::memcpy(out.pixels.data(), formatted->pixels, out.w * out.h * 4);
    SDL_FreeSurface(formatted);

    return true;
}

// Load all pickup textures
void PickupManager::loadPickupAssets() {
    PickupVisual health;
    if (!loadPickupFrame("Assets/Pickups/health.png", health))
        std::cerr << "Failed to load health pickup\n";
    pickupsVisuals[{PickupType::Health, WeaponType::None}] = health;

    PickupVisual armor;
    if (!loadPickupFrame("Assets/Pickups/armor.png", armor))
        std::cerr << "Failed to load armor pickup\n";
    pickupsVisuals[{PickupType::Armor, WeaponType::None}] = armor;

    PickupVisual pistolAmmo;
    if (!loadPickupFrame("Assets/Pickups/p_ammo.png", pistolAmmo))
        std::cerr << "Failed to load ammo pickup\n";
    pickupsVisuals[{PickupType::Ammo, WeaponType::Pistol}] = pistolAmmo;

    PickupVisual shotgunAmmo;
    if (!loadPickupFrame("Assets/Pickups/s_ammo.png", shotgunAmmo))
        std::cerr << "Failed to load ammo pickup\n";
    pickupsVisuals[{PickupType::Ammo, WeaponType::Shotgun}] = shotgunAmmo;

    PickupVisual mgAmmo;
    if (!loadPickupFrame("Assets/Pickups/mg_ammo.png", mgAmmo))
        std::cerr << "Failed to load ammo pickup\n";
    pickupsVisuals[{PickupType::Ammo, WeaponType::Mg}] = mgAmmo;

    PickupVisual pistolWeapon;
    if (!loadPickupFrame("Assets/Pickups/p_gun.png", pistolWeapon))
        std::cerr << "Failed to load gun pickup\n";
    pickupsVisuals[{PickupType::Weapon, WeaponType::Pistol}] = pistolWeapon;

    PickupVisual shotgunWeapon;
    if (!loadPickupFrame("Assets/Pickups/s_gun.png", shotgunWeapon))
        std::cerr << "Failed to load gun pickup\n";
    pickupsVisuals[{PickupType::Weapon, WeaponType::Shotgun}] = shotgunWeapon;

    PickupVisual mgWeapon;
    if (!loadPickupFrame("Assets/Pickups/mg_gun.png", mgWeapon))
        std::cerr << "Failed to load gun pickup\n";
    pickupsVisuals[{PickupType::Weapon, WeaponType::Mg}] = mgWeapon;
}

// Add a pickup to the world
void PickupManager::addPickup(float x, float y, float z, PickupType type, WeaponType id) {
    if (pickups.size() >= MAX_PICKUPS) return;

    Pickup p;
    p.x = x;
    p.y = y;
    p.z = z;
    p.type = type;
    p.id = id;
    p.active = true;
    p.visual = &pickupsVisuals.at({ type, id });

    pickups.push_back(p);
}

// Render pickups in world space
void PickupManager::renderPickups(
    uint32_t* pixels,
    int screenW,
    int screenH,
    const Player& player,
    float* zBuffer,
    Map& map,
    float colWallTop[]
) {
    struct DrawInfo {
        Pickup* pickup;
        float dist;
    };

    DrawInfo drawList[128]; // adjust max pickups if needed
    int count = 0;

    // Collect active pickups and compute distance to player
    for (Pickup& p : pickups) {
        if (!p.active) continue;
        float dx = p.x - player.x;
        float dy = p.y - player.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        drawList[count++] = { &p, dist };
    }

    if (count == 0) return;

    // Sort back-to-front
    std::sort(drawList, drawList + count,
        [](const DrawInfo& a, const DrawInfo& b) { return a.dist > b.dist; }
    );

    // Camera vectors
    float dirX = std::cos(player.angle);
    float dirY = std::sin(player.angle);
    float planeX = -dirY * 0.66f;
    float planeY = dirX * 0.66f;

    for (int i = 0; i < count; i++) {
        Pickup* p = drawList[i].pickup;
        PickupVisual& v = pickupsVisuals.at({p->type, p->id});

        // Compute relative position in world plane (ignore z for horizontal projection)
        float dx = p->x - player.x;
        float dy = p->y - player.y;

        // Transform to camera space
        float invDet = 1.0f / (planeX * dirY - dirX * planeY);
        float transformX = invDet * ( dirY * dx - dirX * dy );
        float transformY = invDet * (-planeY * dx + planeX * dy );

        if (transformY <= 0.05f) continue; // behind camera

        // Screen X position
        int screenX = int((screenW / 2.0f) * (1 + transformX / transformY));

        // Vertical scaling
        float pickupHeight = 0.25f; // world units
        int spriteH = std::max(1, int(screenH / transformY * pickupHeight));
        int spriteW = std::max(1, int(spriteH * (float(v.w) / float(v.h)))); // preserve aspect ratio

        // Vertical position (z offset)
        float pickupBottom = p->z - player.z;
        float pickupTop = p->z + pickupHeight - player.z;

        int drawStartY = int(screenH / 2 - pickupTop / transformY * screenH);
        int drawEndY = int(screenH / 2 - pickupBottom / transformY * screenH);
        int drawStartX = screenX - spriteW / 2;
        int drawEndX = drawStartX + spriteW;

        // Clamp to screen
        drawStartX = std::max(0, drawStartX);
        drawEndX = std::min(screenW - 1, drawEndX);
        drawStartY = std::max(0, drawStartY);
        drawEndY = std::min(screenH - 1, drawEndY);

        int spanW = drawEndX - drawStartX;
        int spanH = drawEndY - drawStartY;
        if (spanW <= 0 || spanH <= 0) continue;

        // Draw pixel lambda
        auto drawPixel = [&](int x, int y) {
            int srcX = std::clamp((x - drawStartX) * v.w / spanW, 0, v.w - 1);
            int srcY = std::clamp((y - drawStartY) * v.h / spanH, 0, v.h - 1);
            uint32_t color = v.pixels[srcY * v.w + srcX];
            if ((color >> 24) == 0) return; // skip transparent
            pixels[y * screenW + x] = color;
        };

        // Draw with z-buffer / wall clipping
        for (int x = drawStartX; x < drawEndX; x++) {
            float wallDepth = zBuffer[x];
            if (wallDepth >= transformY) {
                for (int y = drawStartY; y < drawEndY; y++)
                    drawPixel(x, y);
                continue;
            }

            int wallTopY = int(std::ceil(colWallTop[x]));
            if (wallTopY <= drawStartY) continue;
            int clipEnd = std::min(wallTopY, drawEndY);
            for (int y = drawStartY; y < clipEnd; y++)
                drawPixel(x, y);
        }
    }
}

void PickupManager::applyPickup(Pickup& p, Player& player, Weapon& weapon, AudioManager& audio) {
    switch (p.type) {
        case PickupType::Health:
            player.health = std::min(player.maxHealth, player.health + 50);
            audio.playSFX("heal_pickup");
            break;

        case PickupType::Armor:
            player.armor = std::min(player.maxArmor, player.armor + 25);
            audio.playSFX("armor_pickup");
            break;

        case PickupType::Ammo:
            // p.id stores the WeaponType if pickup is weapon or ammo
            switch (static_cast<WeaponType>(p.id)) {
                case WeaponType::Pistol:
                    weapon.pReserveAmmo = std::min(
                        weapon.pReserveAmmo + 8,
                        weapon.pMaxReserve
                    );
                    audio.playSFX("ammo_pickup");
                    break;

                case WeaponType::Shotgun:
                    weapon.sReserveAmmo = std::min(
                        weapon.sReserveAmmo + 4,
                        weapon.sMaxReserve
                    );
                    audio.playSFX("ammo_pickup");
                    break;

                case WeaponType::Mg:
                    weapon.mgReserveAmmo = std::min(
                        weapon.mgReserveAmmo + 30,
                        weapon.mgMaxReserve
                    );
                    audio.playSFX("ammo_pickup"); 
                    break;

                default:
                    break;
            }
            break;

        case PickupType::Weapon:
            // p.id can store the WeaponType to give
            player.giveItem(static_cast<ItemType>(p.id));
            audio.playSFX("gun_pickup");
            break;
    }

    // Mark pickup as collected
    p.active = false;
}

void PickupManager::update(Player& player, float deltaTime, Weapon& weapon, AudioManager& audio) {
    const float PICKUP_RADIUS = 0.5f; // distance at which player collects the pickup

    for (Pickup& p : pickups) {
        if (!p.active) continue;

        // Compute distance to player in 2D (ignore height for pickup collection)
        float dx = p.x - player.x;
        float dy = p.y - player.y;
        float distSq = dx*dx + dy*dy;

        if (distSq <= PICKUP_RADIUS * PICKUP_RADIUS) {
            // Player is close enough — apply pickup effect
            applyPickup(p, player, weapon, audio);
        }
    }

    // Optional cleanup: remove collected pickups
    pickups.erase(
        std::remove_if(pickups.begin(), pickups.end(),
            [](const Pickup& p) { return !p.active; }),
        pickups.end()
    );
}
