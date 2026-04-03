#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct SBR2Image
{
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels; // RGBA
};

bool capture_screen(SBR2Image &out_image);
bool save_image_as_bmp(const SBR2Image &image, const std::string &path);