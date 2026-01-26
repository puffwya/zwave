#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "WeaponTypes.h"

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1,T2>& p) const noexcept {
        return std::hash<T1>()(p.first) ^ (std::hash<T2>()(p.second) << 1);
    }
};

enum class PickupType {
    Health,
    Armor,
    Ammo,
    Weapon
};

struct PickupVisual {
    int w = 0;
    int h = 0;
    std::vector<uint32_t> pixels;
};

struct Pickup {
    float x = 0, y = 0, z = 0;
    PickupType type;
    WeaponType id = WeaponType::None; // For ammo/gun type
    bool active = true;
    PickupVisual* visual = nullptr;
};

class Player;
class Map;
class Weapon;

class PickupManager {
public:
    static constexpr int MAX_PICKUPS = 64;

    std::vector<Pickup> pickups;
    // key: pair of PickupType + WeaponType
    std::unordered_map<std::pair<PickupType, WeaponType>, PickupVisual, pair_hash> pickupsVisuals;

    // Load all pickup textures
    void loadPickupAssets();

    // Spawn a pickup into the world
    void addPickup(float x, float y, float z, PickupType type, WeaponType id);

    // Render pickups in world space
    void renderPickups(uint32_t* pixels, int screenW, int screenH,
                       const Player& player, float* zBuffer, Map& map, float colWallTop[]);

    void update(Player& player, float deltaTime, Weapon& weapon);

private:
    bool loadPickupFrame(const std::string& path, PickupVisual& out);

    void applyPickup(Pickup& p, Player& player, Weapon& weapon);
};
