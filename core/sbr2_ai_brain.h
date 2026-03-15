#pragma once

#include <cstdint>

#include "sbr2_bomb.h"
#include "sbr2_pathfinder.h"
#include "sbr2_simulator.h"

using i8 = int8_t;
using i32 = int32_t;

class SBR2AIBrain
{
public:
    SBR2AIBrain(
        const SBR2Simulator& simulator,
        const SBR2PathFinder& pathfinder
    );

    SBR2Action decide_next_action(i8 x, i8 y, i32 frame) const;

private:
    const SBR2Simulator& simulator_;
    const SBR2PathFinder& pathfinder_;

    bool will_be_dangerous_soon(i8 x, i8 y, i32 frame) const;
    bool can_place_bomb_and_escape(i8 x, i8 y, i32 frame) const;
};