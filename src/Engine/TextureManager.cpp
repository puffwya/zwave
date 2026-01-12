#include "TextureManager.h"
#include <SDL2/SDL_image.h>
#include <iostream>

bool TextureManager::load(const std::string& name, const std::string& path)
{
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        std::cerr << "Failed to load texture: " << name
                  << " | " << IMG_GetError() << std::endl;
        return false;
    }

    SDL_Surface* formatted =
        SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(surf);

    if (!formatted) return false;

    Texture tex;
    tex.w = formatted->w;
    tex.h = formatted->h;
    tex.pixels.resize(tex.w * tex.h);

    std::memcpy(
        tex.pixels.data(),
        formatted->pixels,
        tex.w * tex.h * 4
    );

    SDL_FreeSurface(formatted);

    textures[name] = std::move(tex);
    return true;
}

const Texture& TextureManager::get(const std::string& name) const
{
    return textures.at(name);
}

