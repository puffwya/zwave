#pragma once
#include <unordered_map>
#include <string>
#include "Texture.h"

class TextureManager {
public:
    bool load(const std::string& name, const std::string& path);
    const Texture& get(const std::string& name) const;

private:
    std::unordered_map<std::string, Texture> textures;
};

