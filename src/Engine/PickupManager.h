#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>

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
    bool active = true;
    PickupVisual* visual = nullptr;
};

class Player;
class Map;

class PickupManager {
public:
    static constexpr int MAX_PICKUPS = 64;

    std::vector<Pickup> pickups;
    std::unordered_map<PickupType, PickupVisual> pickupsVisuals;

    // Load all pickup textures
    void loadPickupAssets();

    // Spawn a pickup into the world
    void addPickup(float x, float y, float z, PickupType type);

    // Render pickups in world space
    void renderPickups(uint32_t* pixels, int screenW, int screenH,
                       const Player& player, float* zBuffer, Map& map, float colWallTop[]);

private:
    bool loadPickupFrame(const std::string& path, PickupVisual& out);
};

