#pragma once

#include <cstdint>
#include <queue>
#include <vector>

#include "sbr2_board.h"
#include "sbr2_simulator.h"

using i8 = int8_t;
using u8 = uint8_t;
using i32 = int32_t;

struct SBR2TimeNode
{
    i8 x;
    i8 y;
    i32 frame;
};

enum class SBR2Action : u8
{
    WAIT,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    PLACE_BOMB,
    PUNCH_RIGHT,
    PUNCH_LEFT,
    PUNCH_UP,
    PUNCH_DOWN,
    KICK_RIGHT,
    KICK_LEFT,
    KICK_UP,
    KICK_DOWN,
    KICK_STOP_UP,
    KICK_STOP_LEFT,
    KICK_STOP_RIGHT,
    KICK_STOP_DOWN
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
    bool found = false;
    i8 target_x = -1;
    i8 target_y = -1;
    SBR2Action first_action = SBR2Action::WAIT;
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

    bool find_escape_action_until(
        i8 start_x,
        i8 start_y,
        i32 start_frame,
        i32 safe_until_frame,
        SBR2EscapeResult& out_result
    ) const;

private:
    const SBR2Board& board_;
    const SBR2Simulator& simulator_;

    bool is_passable(i8 x, i8 y) const;
    bool is_safe(i8 x, i8 y, i32 frame) const;
    bool can_wait(i8 x, i8 y, i32 frame) const;
    bool can_move(i8 x, i8 y, i8 nx, i8 ny, i32 frame) const;

    bool is_safe_continuously(i8 x, i8 y, i32 from_frame, i32 to_frame) const;
};