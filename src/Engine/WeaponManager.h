#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <map>
#include <vector>
#include "WeaponTypes.h"

class Player;

class WeaponManager {
public:

    // Bobbing vars
    float bobTimer = 0.0f;
    float bobAmount = 0.0f;   // final computed sway amount

    bool loadAssets(SDL_Renderer* renderer);
    void update(float delta, const Player& player);
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
