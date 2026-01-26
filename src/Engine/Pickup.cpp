#include "Pickup.h"

void Pickup::apply(Player& player) {
    switch (type) {
        case PickupType::Health:
            player.health = std::min(player.maxHealth, player.health + 25);
            break;

        case PickupType::Armor:
            player.armor = std::min(player.maxArmor, player.armor + 25);
            break;

        case PickupType::Ammo:
            player.ammo += 20;
            break;

        case PickupType::Weapon:
            player.unlockWeapon(/* id */);
            break;
    }

    collected = true;
}
