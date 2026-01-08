#pragma once
#include <cstdint>

struct Texture {
    int width;
    int height;
    uint32_t* pixels; // ARGB8888
};

