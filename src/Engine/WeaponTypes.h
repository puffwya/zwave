#pragma once

enum class WeaponType {
    None,
    Pistol,
    Shotgun,
    Mg
};

class Weapon {
public:
    // Pistol ammo vars
    int pClipAmmo = 8;
    int pClipSize = 8;

    int pReserveAmmo = 32;
    int pMaxReserve = 96;

    // Shotgun ammo vars
    int sClipAmmo = 2;
    int sClipSize = 2;
    
    int sReserveAmmo = 4; 
    int sMaxReserve = 24;

    // Mg ammo vars
    int mgClipAmmo = 30;
    int mgClipSize = 30;
    
    int mgReserveAmmo = 120; 
    int mgMaxReserve = 360;
};
