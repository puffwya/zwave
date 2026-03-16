#include "TextureManager.h"
#include <iostream>
#include <cstring>
#include "../third_party/stb_image_wrapper.h"
#include "../Utils/PathUtils.h"

bool TextureManager::load(const std::string& name, const std::string& path)
{
    // Load surface using wrapper
    std::string fullPath = resolvePath(path);
    SDL_Surface* surface = LoadSurfaceFromPNG(fullPath.c_str());
    if (!surface) {
        std::cerr << "Failed to load texture: " << name << " | " << path << "\n";
        return false;
    }

    Texture tex;
    tex.w = surface->w;
    tex.h = surface->h;
    tex.pixels.resize(tex.w * tex.h);

    // Copy pixels from SDL_Surface into Texture pixel storage
    unsigned char* pixels = static_cast<unsigned char*>(surface->pixels);
    for (int i = 0; i < tex.w * tex.h; ++i) {
        unsigned char r = pixels[i * 4 + 0];
        unsigned char g = pixels[i * 4 + 1];
        unsigned char b = pixels[i * 4 + 2];
        unsigned char a = pixels[i * 4 + 3];
        tex.pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }

    // Free surface (also frees the underlying pixel buffer)
    FreeSurface(surface);

    textures[name] = std::move(tex);
    return true;
}

const Texture& TextureManager::get(const std::string& name) const
{
    return textures.at(name);
}

