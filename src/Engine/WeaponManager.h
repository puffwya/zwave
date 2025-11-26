#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <map>
#include <vector>

enum class WeaponType {
    None,
    Pistol,
    Shotgun,
    // Rifle,
};

class WeaponManager {
public:
    bool loadAssets(SDL_Renderer* renderer);
    void update(float delta);
    SDL_Texture* getCurrentFrame(WeaponType weapon);

    void playShootAnimation(WeaponType weapon);

private:
    struct Animation {
        std::vector<SDL_Texture*> frames;
        float frameTime = 0.1f;   // 100ms per frame
        float timer = 0.0f;
        int current = 0;
        bool playing = false;
    };

    std::map<WeaponType, Animation> animations;
};
