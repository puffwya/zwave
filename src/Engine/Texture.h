#pragma once
#include <cstdint>
#include <vector>

struct Texture {
    int w;
    int h;
    std::vector<uint32_t> pixels;
};

