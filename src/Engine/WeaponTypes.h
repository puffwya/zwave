#pragma once

enum class WeaponType {
    None,
    Pistol,
    Shotgun,
};

class Weapon {
public:
    // Pistol ammo vars
    int pClipAmmo = 8;
    int pClipSize = 8;

    int pReserveAmmo = 32;
    int pMaxReserve = 200;

    // Shotgun ammo vars
    int sClipAmmo = 2;
    int sClipSize = 2;
    
    int sReserveAmmo = 4; 
    int sMaxReserve = 100;

};
