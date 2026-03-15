#pragma once

#include <array>
#include <cstdint>

using i8  = int8_t;
using u8  = uint8_t;
using i32 = int32_t;

class SBR2Board
{
public:
    static constexpr i8 WIDTH  = 13;
    static constexpr i8 HEIGHT = 11;

    enum class CellType : u8
    {
        EMPTY = 0,
        HARD_BLOCK,
        BOMB
    };

private:
    std::array<std::array<CellType, WIDTH>, HEIGHT> cells_;
    i8 enemy_x_ = -1;
    i8 enemy_y_ = -1;
public:
    SBR2Board()
    {
        initialize();
    }

    void initialize()
    {
        for (i8 y = 0; y < HEIGHT; ++y) {
            for (i8 x = 0; x < WIDTH; ++x) {
                cells_[y][x] = CellType::EMPTY;
            }
        }

        // ハードブロック: (x % 2 == 1 && y % 2 == 1)
        for (i8 y = 0; y < HEIGHT; ++y) {
            for (i8 x = 0; x < WIDTH; ++x) {
                if ((x % 2 == 1) && (y % 2 == 1)) {
                    cells_[y][x] = CellType::HARD_BLOCK;
                }
            }
        }
    }

    bool is_inside(i8 x, i8 y) const
    {
        return (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT);
    }

    CellType get_cell(i8 x, i8 y) const
    {
        return cells_[y][x];
    }

    void set_cell(i8 x, i8 y, CellType cell)
    {
        cells_[y][x] = cell;
    }

    bool is_hard_block(i8 x, i8 y) const
    {
        if (!is_inside(x, y)) {
            return false;
        }
        return get_cell(x, y) == CellType::HARD_BLOCK;
    }

    bool is_bomb(i8 x, i8 y) const
    {
        if (!is_inside(x, y)) {
            return false;
        }
        return cells_[y][x] == CellType::BOMB;
    }    

    // bool is_bomb(i8 x, i8 y) const
    // {
    //     if (!is_inside(x, y)) {
    //         return false;
    //     }
    //     return get_cell(x, y) == CellType::BOMB;
    // }

    bool is_empty(i8 x, i8 y) const
    {
        if (!is_inside(x, y)) {
            return false;
        }
        return get_cell(x, y) == CellType::EMPTY;
    }

    bool is_passable(i8 x, i8 y) const
    {
        if (!is_inside(x, y)) {
            return false;
        }

        return get_cell(x, y) == CellType::EMPTY;
    }

    void set_enemy_position(i8 x, i8 y)
    {
        enemy_x_ = x;
        enemy_y_ = y;
    }

    i8 enemy_x() const
    {
        return enemy_x_;
    }

    i8 enemy_y() const
    {
        return enemy_y_;
    }

};