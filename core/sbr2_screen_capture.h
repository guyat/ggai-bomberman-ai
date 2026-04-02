#pragma once

#include <vector>
#include <cstdint>

struct SBR2Image
{
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels; // RGBA
};

bool capture_screen(SBR2Image &out_image);