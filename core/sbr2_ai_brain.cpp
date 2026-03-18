#include "sbr2_ai_brain.h"

#include <array>
#include <cmath>
#include <queue>

SBR2AIBrain::SBR2AIBrain(
    const SBR2Simulator &simulator,
    const SBR2PathFinder &pathfinder)
    : simulator_(simulator),
      pathfinder_(pathfinder),
      settings_{20}
{
}

SBR2AIBrain::SBR2AIBrain(
    const SBR2Simulator &simulator,
    const SBR2PathFinder &pathfinder,
    const SBR2AIBrainSettings &settings)
    : simulator_(simulator),
      pathfinder_(pathfinder),
      settings_(settings)
{
}

int SBR2AIBrain::ai_level() const
{
    return normalized_ai_level();
}

int SBR2AIBrain::normalized_ai_level() const
{
    if (settings_.ai_level < 1)
    {
        return 1;
    }

    if (settings_.ai_level > 20)
    {
        return 20;
    }

    return settings_.ai_level;
}

int SBR2AIBrain::bomb_cooldown_frames() const
{
    int level = normalized_ai_level();
    SBR2AIStyle s = style();

    if (s == SBR2AIStyle::Careful)
    {
        if (level <= 5)
            return 70;
        if (level <= 10)
            return 55;
        if (level <= 15)
            return 40;
        return 30;
    }

    if (s == SBR2AIStyle::Tricky)
    {
        if (level <= 5)
            return 60;
        if (level <= 10)
            return 40;
        if (level <= 15)
            return 28;
        return 18;
    }

    // Aggressive
    if (level <= 5)
        return 55;
    if (level <= 10)
        return 40;
    if (level <= 15)
        return 26;
    return 16;
}

bool SBR2AIBrain::level_allows_straight_kill() const
{
    return normalized_ai_level() >= 6;
}

bool SBR2AIBrain::level_allows_one_step_prediction() const
{
    return normalized_ai_level() >= 10;
}

bool SBR2AIBrain::level_allows_trap() const
{
    return normalized_ai_level() >= 15;
}

int SBR2AIBrain::trap_survivable_cell_threshold() const
{
    int level = normalized_ai_level();
    SBR2AIStyle s = style();

    if (level < 15)
    {
        return -1;
    }

    if (s == SBR2AIStyle::Careful)
    {
        if (level >= 19)
            return 1;
        return 0;
    }

    if (s == SBR2AIStyle::Aggressive)
    {
        if (level >= 19)
            return 2;
        return 1;
    }

    // Tricky
    if (level >= 19)
        return 4;
    if (level >= 17)
        return 3;
    return 2;
}

bool SBR2AIBrain::is_move_action(SBR2Action action) const
{
    switch (action)
    {
    case SBR2Action::UP:
    case SBR2Action::DOWN:
    case SBR2Action::LEFT:
    case SBR2Action::RIGHT:
        return true;
    default:
        return false;
    }
}

int SBR2AIBrain::same_direction_reposition_limit() const
{
    int level = normalized_ai_level();

    if (style() == SBR2AIStyle::Careful)
    {
        if (level >= 15)
            return 2;
        return 1;
    }

    if (style() == SBR2AIStyle::Tricky)
    {
        if (level >= 15)
            return 3;
        return 2;
    }

    // Aggressive
    if (level >= 15)
        return 4;
    return 3;
}

SBR2Action SBR2AIBrain::remember_reposition_action(SBR2Action action) const
{
    if (!is_move_action(action))
    {
        last_reposition_action_ = SBR2Action::WAIT;
        same_direction_reposition_count_ = 0;
        return action;
    }

    if (action == last_reposition_action_)
    {
        ++same_direction_reposition_count_;
    }
    else
    {
        last_reposition_action_ = action;
        same_direction_reposition_count_ = 1;
    }

    return action;
}

SBR2Action SBR2AIBrain::reset_reposition_state_and_return(SBR2Action action) const
{
    last_reposition_action_ = SBR2Action::WAIT;
    same_direction_reposition_count_ = 0;
    held_reposition_action_ = SBR2Action::WAIT;
    held_reposition_until_frame_ = -1;
    return action;
}

int SBR2AIBrain::reposition_hold_extra_frames() const
{
    return 1;
}

SBR2Action SBR2AIBrain::apply_reposition_hold(SBR2Action action, i32 frame) const
{
    if (!is_move_action(action))
    {
        held_reposition_action_ = SBR2Action::WAIT;
        held_reposition_until_frame_ = -1;
        return action;
    }

    // 新しく方向が変わった瞬間だけ、次の1回分だけ維持する
    if (action != last_reposition_action_)
    {
        held_reposition_action_ = action;
        held_reposition_until_frame_ = frame + reposition_hold_extra_frames();
    }

    return action;
}

bool SBR2AIBrain::will_be_dangerous_soon(i8 x, i8 y, i32 frame) const
{
    const int LOOKAHEAD = 40;

    for (int t = 0; t <= LOOKAHEAD; ++t)
    {
        int check_frame = frame + t;

        if (check_frame >= SBR2Simulator::MAX_SIMULATION_FRAMES)
        {
            break;
        }

        if (simulator_.is_danger(check_frame, x, y))
        {
            return true;
        }
    }

    return false;
}

bool SBR2AIBrain::can_place_bomb_and_escape(i8 x, i8 y, i32 frame) const
{
    if (!simulator_.board().is_inside(x, y))
    {
        return false;
    }

    if (simulator_.board().is_hard_block(x, y))
    {
        return false;
    }

    if (simulator_.is_danger(frame, x, y))
    {
        return false;
    }

    if (simulator_.board().is_bomb(x, y))
    {
        return false;
    }

    SBR2Simulator sim_after_place = simulator_;
    sim_after_place.add_bomb(x, y, frame);
    sim_after_place.simulate();

    SBR2PathFinder pathfinder_after(
        sim_after_place.board(),
        sim_after_place);

    SBR2EscapeResult result{};

    const i32 safe_until_frame =
        frame + SBR2Bomb::EXPLOSION_TIMER + SBR2Bomb::FIRE_DURATION;

    return pathfinder_after.find_escape_action_until(
        x,
        y,
        frame,
        safe_until_frame,
        result);
}

bool SBR2AIBrain::can_hit_enemy_in_straight_line(i8 x, i8 y, i8 ex, i8 ey) const
{
    const SBR2Board &board = simulator_.board();

    if (ex < 0 || ey < 0)
    {
        return false;
    }

    if (!board.is_inside(ex, ey))
    {
        return false;
    }

    if (x == ex && y == ey)
    {
        return false;
    }

    if (x != ex && y != ey)
    {
        return false;
    }

    int dist = std::abs(ex - x) + std::abs(ey - y);
    if (dist > SBR2Bomb::RANGE)
    {
        return false;
    }

    if (x == ex)
    {
        int step = (ey > y) ? 1 : -1;

        for (int cy = y + step; cy != ey; cy += step)
        {
            if (board.is_hard_block(x, cy))
            {
                return false;
            }
        }

        return true;
    }

    if (y == ey)
    {
        int step = (ex > x) ? 1 : -1;

        for (int cx = x + step; cx != ex; cx += step)
        {
            if (board.is_hard_block(cx, y))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool SBR2AIBrain::can_hit_enemy_now_or_after_one_step(
    i8 x,
    i8 y,
    bool allow_one_step_prediction) const
{
    const SBR2Board &board = simulator_.board();

    i8 ex = board.enemy_x();
    i8 ey = board.enemy_y();

    if (ex < 0 || ey < 0)
    {
        return false;
    }

    if (can_hit_enemy_in_straight_line(x, y, ex, ey))
    {
        return true;
    }

    if (!allow_one_step_prediction)
    {
        return false;
    }

    const std::array<i8, 4> dx = {0, 0, -1, 1};
    const std::array<i8, 4> dy = {-1, 1, 0, 0};

    for (int i = 0; i < 4; ++i)
    {
        i8 nx = ex + dx[i];
        i8 ny = ey + dy[i];

        if (!board.is_inside(nx, ny))
        {
            continue;
        }

        if (!board.is_passable(nx, ny))
        {
            continue;
        }

        if (can_hit_enemy_in_straight_line(x, y, nx, ny))
        {
            return true;
        }
    }

    return false;
}

int SBR2AIBrain::count_enemy_survivable_cells_after_bomb(i8 self_x, i8 self_y, i32 frame) const
{
    const SBR2Board &board = simulator_.board();

    i8 ex = board.enemy_x();
    i8 ey = board.enemy_y();

    if (ex < 0 || ey < 0)
    {
        return 999;
    }

    if (!board.is_inside(ex, ey))
    {
        return 999;
    }

    SBR2Simulator sim_after_place = simulator_;
    sim_after_place.add_bomb(self_x, self_y, frame);
    sim_after_place.simulate();

    const i32 survive_start = frame + SBR2Bomb::EXPLOSION_TIMER;
    const i32 survive_end =
        frame + SBR2Bomb::EXPLOSION_TIMER + SBR2Bomb::FIRE_DURATION;

    struct Node
    {
        i8 x;
        i8 y;
        i32 time;
    };

    bool visited[SBR2Board::HEIGHT][SBR2Board::WIDTH][160] = {};
    bool survivable_cell_mark[SBR2Board::HEIGHT][SBR2Board::WIDTH] = {};

    std::queue<Node> q;
    q.push({ex, ey, frame});
    visited[ey][ex][0] = true;

    const std::array<i8, 5> dx = {0, 0, -1, 1, 0};
    const std::array<i8, 5> dy = {-1, 1, 0, 0, 0};

    int survivable_count = 0;

    while (!q.empty())
    {
        Node cur = q.front();
        q.pop();

        // 爆発開始時点までに到達できる場所だけを見る
        if (cur.time > survive_start)
        {
            continue;
        }

        bool safe_for_whole_fire_window = true;

        // このマスが、爆発開始〜爆風終了まで生存可能か
        for (i32 t = survive_start; t <= survive_end; ++t)
        {
            if (sim_after_place.is_danger(t, cur.x, cur.y))
            {
                safe_for_whole_fire_window = false;
                break;
            }
        }

        if (safe_for_whole_fire_window)
        {
            if (!survivable_cell_mark[cur.y][cur.x])
            {
                survivable_cell_mark[cur.y][cur.x] = true;
                ++survivable_count;
            }
        }

        if (cur.time == survive_start)
        {
            continue;
        }

        for (int dir = 0; dir < 5; ++dir)
        {
            i8 nx = cur.x + dx[dir];
            i8 ny = cur.y + dy[dir];
            i32 nt = cur.time + 1;

            if (!board.is_inside(nx, ny))
            {
                continue;
            }

            if (!board.is_passable(nx, ny))
            {
                continue;
            }

            if (sim_after_place.is_danger(nt, nx, ny))
            {
                continue;
            }

            int dt = nt - frame;
            if (dt < 0 || dt >= 160)
            {
                continue;
            }

            if (visited[ny][nx][dt])
            {
                continue;
            }

            visited[ny][nx][dt] = true;
            q.push({nx, ny, nt});
        }
    }

    return survivable_count;
}

bool SBR2AIBrain::is_trap_possible(i8 self_x, i8 self_y, i32 frame) const
{
    if (!can_place_bomb_and_escape(self_x, self_y, frame))
    {
        return false;
    }

    int threshold = trap_survivable_cell_threshold();
    if (threshold < 0)
    {
        return false;
    }

    int survivable_cells =
        count_enemy_survivable_cells_after_bomb(self_x, self_y, frame);

    // Tricky 高レベルは、直線上にいなくても
    // 「逃げ道がかなり少ない」だけで置きに行く
    if (style() == SBR2AIStyle::Tricky && normalized_ai_level() >= 18)
    {
        return survivable_cells <= threshold;
    }

    return survivable_cells <= threshold;
}

SBR2Action SBR2AIBrain::decide_next_action(i8 x, i8 y, i32 frame) const
{
    SBR2EscapeResult result{};
    bool can_escape = pathfinder_.find_escape_action(x, y, frame, result);

    if (!can_escape || will_be_dangerous_soon(x, y, frame))
    {
        if (can_escape)
        {
            return reset_reposition_state_and_return(result.first_action);
        }
        return reset_reposition_state_and_return(SBR2Action::WAIT);
    }

    SBR2AIStyle s = style();
    bool allow_risky =
        (s == SBR2AIStyle::Aggressive) ||
        (s == SBR2AIStyle::Tricky && normalized_ai_level() >= 18);

    bool trap_first =
        (s == SBR2AIStyle::Tricky && normalized_ai_level() >= 16);

    bool tricky_trap_second_chance =
        (s == SBR2AIStyle::Tricky && normalized_ai_level() >= 18);

    if (frame - last_bomb_frame_ > bomb_cooldown_frames())
    {
        // 高レベルTrickyは trap を先に見る
        if (trap_first && level_allows_trap())
        {
            if (is_trap_possible(x, y, frame))
            {
                last_bomb_frame_ = frame;
                return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
            }
        }

        // 直線キル
        if (level_allows_straight_kill())
        {
            if (can_hit_enemy_now_or_after_one_step(
                    x, y, level_allows_one_step_prediction()))
            {
                bool safe = can_place_bomb_and_escape(x, y, frame);

                if (style() == SBR2AIStyle::Careful)
                {
                    safe = can_place_bomb_and_escape_strict(x, y, frame);
                }

                if (safe || allow_risky)
                {
                    last_bomb_frame_ = frame;
                    return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                }
            }
        }

        // trap（通常順）
        if (!trap_first && level_allows_trap())
        {
            if (is_trap_possible(x, y, frame))
            {
                if (style() == SBR2AIStyle::Careful)
                {
                    if (!can_place_bomb_and_escape_strict(x, y, frame))
                    {
                        return reset_reposition_state_and_return(SBR2Action::WAIT);
                    }
                }

                last_bomb_frame_ = frame;
                return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
            }
        }

        // 高レベルTrickyだけ、最後にもう一度 trap を見る
        if (tricky_trap_second_chance && level_allows_trap())
        {
            if (is_trap_possible(x, y, frame))
            {
                last_bomb_frame_ = frame;
                return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
            }
        }
    }

    // ===== 人間っぽい動き：安全なら軽く位置調整 =====
    if (normalized_ai_level() >= 10 &&
        style() != SBR2AIStyle::Careful &&
        !will_be_dangerous_soon(x, y, frame))
    {
        i8 enemy_x = simulator_.board().enemy_x();
        i8 enemy_y = simulator_.board().enemy_y();
        int dist = std::abs(enemy_x - x) + std::abs(enemy_y - y);

        if (dist >= 4)
        {
            if (settings_.style == SBR2AIStyle::Aggressive)
            {
                return remember_reposition_action(
                    apply_reposition_hold(move_toward_enemy(x, y, frame), frame));
            }

            if (settings_.style == SBR2AIStyle::Tricky)
            {
                if (enemy_x != x && enemy_y != y)
                {
                    return remember_reposition_action(
                        apply_reposition_hold(move_toward_enemy(x, y, frame), frame));
                }
            }

            if (settings_.style == SBR2AIStyle::Careful)
            {
                if (dist >= 6)
                {
                    return remember_reposition_action(
                        apply_reposition_hold(move_toward_enemy(x, y, frame), frame));
                }
            }
        }
    }

    return reset_reposition_state_and_return(SBR2Action::WAIT);
}

SBR2Action SBR2AIBrain::decide_reposition_action_for_test(
    i8 x, i8 y, i32 frame) const
{
    return remember_reposition_action(
        apply_reposition_hold(move_toward_enemy(x, y, frame), frame));
}

bool SBR2AIBrain::can_step_to(i8 x, i8 y) const
{
    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    if (!board.is_passable(x, y))
    {
        return false;
    }

    return true;
}

SBR2Action SBR2AIBrain::fallback_safe_step(i8 self_x, i8 self_y) const
{
    if (can_step_to(self_x, self_y - 1))
        return SBR2Action::UP;
    if (can_step_to(self_x, self_y + 1))
        return SBR2Action::DOWN;
    if (can_step_to(self_x - 1, self_y))
        return SBR2Action::LEFT;
    if (can_step_to(self_x + 1, self_y))
        return SBR2Action::RIGHT;

    return SBR2Action::WAIT;
}

SBR2Action SBR2AIBrain::move_toward_enemy(i8 self_x, i8 self_y, i32 frame) const
{
    i8 enemy_x = simulator_.board().enemy_x();
    i8 enemy_y = simulator_.board().enemy_y();

    int dx = enemy_x - self_x;
    int dy = enemy_y - self_y;

    SBR2Action first = SBR2Action::WAIT;
    SBR2Action second = SBR2Action::WAIT;

    if (std::abs(dx) > std::abs(dy))
    {
        if (dx > 0)
            first = SBR2Action::RIGHT;
        else if (dx < 0)
            first = SBR2Action::LEFT;

        if (dy > 0)
            second = SBR2Action::DOWN;
        else if (dy < 0)
            second = SBR2Action::UP;
    }
    else
    {
        if (dy > 0)
            first = SBR2Action::DOWN;
        else if (dy < 0)
            first = SBR2Action::UP;

        if (dx > 0)
            second = SBR2Action::RIGHT;
        else if (dx < 0)
            second = SBR2Action::LEFT;
    }

    auto can_use_action = [&](SBR2Action action) -> bool
    {
        switch (action)
        {
        case SBR2Action::UP:
            return can_step_to(self_x, self_y - 1);
        case SBR2Action::DOWN:
            return can_step_to(self_x, self_y + 1);
        case SBR2Action::LEFT:
            return can_step_to(self_x - 1, self_y);
        case SBR2Action::RIGHT:
            return can_step_to(self_x + 1, self_y);
        default:
            return false;
        }
    };

    // 壁際や境界で毎フレーム方針が揺れすぎないよう、
    // 直前に決めた再配置方向を短時間だけ維持する
    if (frame <= held_reposition_until_frame_ &&
        is_move_action(held_reposition_action_) &&
        can_use_action(held_reposition_action_))
    {
        return held_reposition_action_;
    }

    bool avoid_same_direction_too_much =
        is_move_action(first) &&
        first == last_reposition_action_ &&
        same_direction_reposition_count_ >= same_direction_reposition_limit();

    if (avoid_same_direction_too_much && can_use_action(second))
    {
        return second;
    }

    if (can_use_action(first))
    {
        return first;
    }

    if (can_use_action(second))
    {
        return second;
    }

    return fallback_safe_step(self_x, self_y);
}

bool SBR2AIBrain::can_place_bomb_and_escape_strict(i8 x, i8 y, i32 frame) const
{
    if (!can_place_bomb_and_escape(x, y, frame))
    {
        return false;
    }

    SBR2EscapeResult result{};
    bool can_escape = pathfinder_.find_escape_action(x, y, frame, result);

    if (!can_escape)
    {
        return false;
    }

    // ★ここが重要：逃げ先を見る
    i8 ex = result.target_x;
    i8 ey = result.target_y;

    int safe_frames = 0;

    for (int t = 1; t <= 20; ++t)
    {
        int check_frame = frame + t;

        if (!simulator_.is_danger(check_frame, ex, ey))
        {
            safe_frames++;
        }
    }

    return safe_frames >= 10;
}