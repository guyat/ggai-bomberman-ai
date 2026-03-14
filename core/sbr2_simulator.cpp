#include "sbr2_simulator.h"

void SBR2Simulator::clear_danger_map()
{
    for (i32 t = 0; t < MAX_SIMULATION_FRAMES; ++t) {
        for (i8 y = 0; y < SBR2Board::HEIGHT; ++y) {
            for (i8 x = 0; x < SBR2Board::WIDTH; ++x) {
                danger_map_[t][y][x] = false;
            }
        }
    }
}

void SBR2Simulator::mark_fire_cell(i8 x, i8 y, i32 start_frame, i32 duration)
{
    if (!board_.is_inside(x, y)) {
        return;
    }

    for (i32 t = start_frame; t < start_frame + duration; ++t) {
        if (t < 0 || t >= MAX_SIMULATION_FRAMES) {
            continue;
        }
        danger_map_[t][y][x] = true;
    }

    try_chain_bombs_hit_by_fire(x, y, start_frame);
}

void SBR2Simulator::try_chain_bombs_hit_by_fire(i8 x, i8 y, i32 frame)
{
    for (auto& bomb : bombs_) {
        if (bomb.exploded) {
            continue;
        }

        if (bomb.x != x || bomb.y != y) {
            continue;
        }

        i32 scheduled_frame = frame + 10;

        if (!bomb.chain_scheduled || scheduled_frame < bomb.chain_frame) {
            bomb.chain_scheduled = true;
            bomb.chain_frame = scheduled_frame;
        }
    }
}

void SBR2Simulator::explode_bomb(i32 bomb_index, i32 frame)
{
    SBR2Bomb& bomb = bombs_[bomb_index];

    if (bomb.exploded) {
        return;
    }

    bomb.exploded = true;
    board_.set_cell(bomb.x, bomb.y, SBR2Board::CellType::EMPTY);

    // 中心マス
    mark_fire_cell(bomb.x, bomb.y, frame, SBR2Bomb::FIRE_DURATION);

    constexpr i8 dx[4] = { 1, -1, 0, 0 };
    constexpr i8 dy[4] = { 0, 0, 1, -1 };

    for (i32 dir = 0; dir < 4; ++dir) {
        for (i32 dist = 1; dist <= SBR2Bomb::RANGE; ++dist) {
            i8 nx = bomb.x + dx[dir] * dist;
            i8 ny = bomb.y + dy[dir] * dist;

            if (!board_.is_inside(nx, ny)) {
                break;
            }

            // ハードブロックのマスには爆風が入らず、そこで止まる
            if (board_.is_hard_block(nx, ny)) {
                break;
            }

            mark_fire_cell(nx, ny, frame, SBR2Bomb::FIRE_DURATION);
        }
    }
}

void SBR2Simulator::simulate()
{
    clear_danger_map();

    for (i32 frame = 0; frame < MAX_SIMULATION_FRAMES; ++frame) {
        bool exploded_any = true;

        while (exploded_any) {
            exploded_any = false;

            for (i32 i = 0; i < static_cast<i32>(bombs_.size()); ++i) {
                SBR2Bomb& bomb = bombs_[i];

                if (bomb.exploded) {
                    continue;
                }

                bool should_explode = false;

                if (bomb.explosion_frame() == frame) {
                    should_explode = true;
                }

                if (bomb.chain_scheduled && bomb.chain_frame == frame) {
                    should_explode = true;
                }

                if (should_explode) {
                    explode_bomb(i, frame);
                    exploded_any = true;
                }
            }
        }
    }
}