#pragma once
#include <vector>
#include <cstdint>

struct PickupVisual {
    int w = 0;
    int h = 0;
    std::vector<uint32_t> pixels;
};

