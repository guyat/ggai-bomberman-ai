#pragma once

#include <array>
#include <vector>

#include "sbr2_board.h"
#include "sbr2_bomb.h"

using i8  = int8_t;
using i32 = int32_t;

class SBR2Simulator
{
public:
    static constexpr i32 MAX_SIMULATION_FRAMES = 256;

private:
    SBR2Board board_;
    std::vector<SBR2Bomb> bombs_;

    // danger_map_[t][y][x] == true なら
    // フレーム t に座標 (x, y) が爆風で危険
    std::array<
        std::array<
            std::array<bool, SBR2Board::WIDTH>,
            SBR2Board::HEIGHT
        >,
        MAX_SIMULATION_FRAMES
    > danger_map_{};

public:
    SBR2Simulator()
    {
        clear_danger_map();
    }

    void clear()
    {
        board_.initialize();
        bombs_.clear();
        clear_danger_map();
    }

    void add_bomb(i8 x, i8 y, i32 placed_frame)
    {
        bombs_.emplace_back(x, y, placed_frame);
        board_.set_cell(x, y, SBR2Board::CellType::BOMB);
    }

    const SBR2Board& board() const
    {
        return board_;
    }

    const std::vector<SBR2Bomb>& bombs() const
    {
        return bombs_;
    }

    bool is_danger(i32 frame, i8 x, i8 y) const
    {
        if (frame < 0 || frame >= MAX_SIMULATION_FRAMES) {
            return false;
        }
        if (!board_.is_inside(x, y)) {
            return false;
        }
        return danger_map_[frame][y][x];
    }

    void simulate();

private:
    void clear_danger_map();
    void explode_bomb(i32 bomb_index, i32 frame);
    void mark_fire_cell(i8 x, i8 y, i32 start_frame, i32 duration);
    void try_chain_bombs_hit_by_fire(i8 x, i8 y, i32 frame);
};