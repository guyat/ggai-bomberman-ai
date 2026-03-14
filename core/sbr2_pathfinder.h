#pragma once

#include <queue>
#include <vector>

#include "sbr2_board.h"
#include "sbr2_simulator.h"

using i8  = int8_t;
using i32 = int32_t;

struct SBR2TimeNode
{
    i8 x;
    i8 y;
    i32 frame;
};

enum class SBR2Action : u8
{
    WAIT = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

struct SBR2RouteNode
{
    i8 x;
    i8 y;
    i32 frame;

    i32 parent_index;
    SBR2Action action_from_parent;
};

struct SBR2EscapeResult
{
    bool found;
    i8 target_x;
    i8 target_y;
    SBR2Action first_action;
};

class SBR2PathFinder
{
public:
    static constexpr i32 MOVE_FRAMES = 11;

    SBR2PathFinder(const SBR2Board& board, const SBR2Simulator& simulator);

    bool can_reach_safe_cell(i8 start_x, i8 start_y, i32 start_frame) const;

    bool find_safe_cell(
        i8 start_x,
        i8 start_y,
        i32 start_frame,
        i8& out_x,
        i8& out_y
    ) const;

        bool find_escape_action(
        i8 start_x,
        i8 start_y,
        i32 start_frame,
        SBR2EscapeResult& out_result
    ) const;

private:
    const SBR2Board& board_;
    const SBR2Simulator& simulator_;

    bool is_passable(i8 x, i8 y) const;
    bool is_safe(i8 x, i8 y, i32 frame) const;
    bool can_wait(i8 x, i8 y, i32 frame) const;
    bool can_move(i8 x, i8 y, i8 nx, i8 ny, i32 frame) const;
};