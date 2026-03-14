#pragma once

#include <cstdint>

using i8  = int8_t;
using i32 = int32_t;

class SBR2Bomb
{
public:
    i8 x;
    i8 y;

    i32 placed_frame;

    static constexpr i32 EXPLOSION_TIMER = 153;
    static constexpr i32 FIRE_DURATION   = 40;
    static constexpr i32 RANGE           = 8;

    bool exploded = false;

    // 誘爆用
    bool chain_scheduled = false;
    i32 chain_frame = -1;

public:

    SBR2Bomb(i8 x_, i8 y_, i32 frame)
        : x(x_), y(y_), placed_frame(frame)
    {
    }

    i32 explosion_frame() const
    {
        return placed_frame + EXPLOSION_TIMER;
    }
};