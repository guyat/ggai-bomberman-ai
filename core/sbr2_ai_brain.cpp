#include "sbr2_ai_brain.h"

#include <array>
#include <cmath>
#include <queue>
#include <iostream>
#include <string>
std::string g_last_bomb_reason;

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

int SBR2AIBrain::same_direction_reposition_limit(i8 x, i8 y) const
{
    int level = normalized_ai_level();
    int limit = 3;

    if (style() == SBR2AIStyle::Careful)
    {
        if (level >= 15)
            limit = 2;
        else
            limit = 1;
    }
    else if (style() == SBR2AIStyle::Tricky)
    {
        if (level >= 15)
            limit = 3;
        else
            limit = 2;
    }
    else
    {
        // Aggressive
        if (level >= 15)
            limit = 4;
        else
            limit = 3;
    }

    // 敵が近いほど、同じ方向への連打を早めに抑える
    i8 enemy_x = simulator_.board().enemy_x();
    i8 enemy_y = simulator_.board().enemy_y();
    int enemy_dist = std::abs(enemy_x - x) + std::abs(enemy_y - y);

    if (enemy_dist <= 4 && limit > 1)
    {
        --limit;
    }

    if (enemy_dist <= 2 && limit > 1)
    {
        --limit;
    }

    return limit;
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

int SBR2AIBrain::reposition_hold_extra_frames(i8 x, i8 y) const
{
    const int min_x = 0;
    const int max_x = 12;
    const int min_y = 0;
    const int max_y = 10;

    int dist_left = x - min_x;
    int dist_right = max_x - x;
    int dist_up = y - min_y;
    int dist_down = max_y - y;

    int min_dist_to_wall = std::min(
        std::min(dist_left, dist_right),
        std::min(dist_up, dist_down));

    int hold_frames = 1;

    // 壁際ほど長めに保持
    if (min_dist_to_wall <= 1)
    {
        hold_frames = 3;
    }
    else if (min_dist_to_wall <= 2)
    {
        hold_frames = 2;
    }

    // ただし敵が近いなら、反応を鈍くしすぎないように1段短くする
    i8 enemy_x = simulator_.board().enemy_x();
    i8 enemy_y = simulator_.board().enemy_y();
    int enemy_dist = std::abs(enemy_x - x) + std::abs(enemy_y - y);

    if (enemy_dist <= 4 && hold_frames > 1)
    {
        --hold_frames;
    }

    return hold_frames;
}

SBR2Action SBR2AIBrain::apply_reposition_hold(
    SBR2Action action, i8 x, i8 y, i32 frame) const
{
    if (!is_move_action(action))
    {
        held_reposition_action_ = SBR2Action::WAIT;
        held_reposition_until_frame_ = -1;
        return action;
    }

    // すでに保持中なら、その期限内は維持
    if (frame < held_reposition_until_frame_ &&
        is_move_action(held_reposition_action_))
    {
        return held_reposition_action_;
    }

    // 新しく方向が変わったときだけ保持開始
    if (action != held_reposition_action_)
    {
        held_reposition_action_ = action;
        held_reposition_until_frame_ =
            frame + reposition_hold_extra_frames(x, y);
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

    std::array<i8, 4> cand_x{};
    std::array<i8, 4> cand_y{};
    int cand_count = 0;

    auto add_candidate = [&](i8 nx, i8 ny)
    {
        for (int i = 0; i < cand_count; ++i)
        {
            if (cand_x[i] == nx && cand_y[i] == ny)
            {
                return;
            }
        }

        cand_x[cand_count] = nx;
        cand_y[cand_count] = ny;
        ++cand_count;
    };

    // まずは「自分から離れる方向」を優先して予測する
    if (ex > x)
    {
        add_candidate(ex + 1, ey);
    }
    else if (ex < x)
    {
        add_candidate(ex - 1, ey);
    }

    if (ey > y)
    {
        add_candidate(ex, ey + 1);
    }
    else if (ey < y)
    {
        add_candidate(ex, ey - 1);
    }

    // その後に残りの1歩候補も全部見る
    add_candidate(ex, ey - 1);
    add_candidate(ex, ey + 1);
    add_candidate(ex - 1, ey);
    add_candidate(ex + 1, ey);

    for (int i = 0; i < cand_count; ++i)
    {
        i8 nx = cand_x[i];
        i8 ny = cand_y[i];

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

    const SBR2Board &board = simulator_.board();
    i8 enemy_x = board.enemy_x();
    i8 enemy_y = board.enemy_y();

    if (enemy_x < 0 || enemy_y < 0)
    {
        return false;
    }

    int enemy_dist = std::abs(enemy_x - self_x) + std::abs(enemy_y - self_y);

    int survivable_cells =
        count_enemy_survivable_cells_after_bomb(self_x, self_y, frame);

    // 近距離なら少しだけ強気にトラップ判定を広げる
    if (enemy_dist <= 3)
    {
        survivable_cells -= 1;
    }

    // Tricky 高レベルは、近距離ならさらに少しだけ trap を狙いやすくする
    if (style() == SBR2AIStyle::Tricky && normalized_ai_level() >= 18)
    {
        if (enemy_dist <= 2)
        {
            survivable_cells -= 1;
        }

        return survivable_cells <= threshold;
    }

    return survivable_cells <= threshold;
}

bool SBR2AIBrain::can_use_surrounded_punch_escape_right(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 右隣に爆弾が無いならパンチ対象外
    if (!board.is_bomb(x + 1, y))
    {
        return false;
    }

    // 囲われ判定:
    // 上・左・下が通れないなら「右へ盤面を崩したい状況」とみなす
    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(up_blocked && left_blocked && down_blocked))
    {
        return false;
    }

    // 条件1:
    // 右に爆弾が2個以上連なっている
    if (board.is_bomb(x + 2, y))
    {
        return true;
    }

    // 条件2:
    // 右に爆弾1個 + その1つ先が壁・敵・非通行
    const i8 next_x = x + 2;
    const i8 next_y = y;

    if (!board.is_inside(next_x, next_y))
    {
        return true;
    }

    if (board.enemy_x() == next_x && board.enemy_y() == next_y)
    {
        return true;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return true;
    }

    return false;
}

bool SBR2AIBrain::can_use_surrounded_punch_escape_left(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 左隣に爆弾が無いならパンチ対象外
    if (!board.is_bomb(x - 1, y))
    {
        return false;
    }

    // 囲われ判定:
    // 上・右・下が通れないなら「左へ盤面を崩したい状況」とみなす
    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(up_blocked && right_blocked && down_blocked))
    {
        return false;
    }

    // 条件1:
    // 左に爆弾が2個以上連なっている
    if (board.is_bomb(x - 2, y))
    {
        return true;
    }

    // 条件2:
    // 左に爆弾1個 + その1つ先が壁・敵・非通行
    const i8 next_x = x - 2;
    const i8 next_y = y;

    if (!board.is_inside(next_x, next_y))
    {
        return true;
    }

    if (board.enemy_x() == next_x && board.enemy_y() == next_y)
    {
        return true;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return true;
    }

    return false;
}

bool SBR2AIBrain::can_use_surrounded_punch_escape_up(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 上隣に爆弾が無いならパンチ対象外
    if (!board.is_bomb(x, y - 1))
    {
        return false;
    }

    // 囲われ判定:
    // 左・右・下が通れないなら「上へ盤面を崩したい状況」とみなす
    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(left_blocked && right_blocked && down_blocked))
    {
        return false;
    }

    // 条件1:
    // 上に爆弾が2個以上連なっている
    if (board.is_bomb(x, y - 2))
    {
        return true;
    }

    // 条件2:
    // 上に爆弾1個 + その1つ先が壁・敵・非通行
    const i8 next_x = x;
    const i8 next_y = y - 2;

    if (!board.is_inside(next_x, next_y))
    {
        return true;
    }

    if (board.enemy_x() == next_x && board.enemy_y() == next_y)
    {
        return true;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return true;
    }

    return false;
}

bool SBR2AIBrain::can_use_surrounded_punch_escape_down(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 下に爆弾が無いなら対象外
    if (!board.is_bomb(x, y + 1))
    {
        return false;
    }

    // 囲われ判定（上・左・右が詰まってる）
    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    if (!(up_blocked && left_blocked && right_blocked))
    {
        return false;
    }

    // 2連爆弾
    if (board.is_bomb(x, y + 2))
    {
        return true;
    }

    // 1個 + 先が壁/敵/非通行
    const i8 next_x = x;
    const i8 next_y = y + 2;

    if (!board.is_inside(next_x, next_y))
    {
        return true;
    }

    if (board.enemy_x() == next_x && board.enemy_y() == next_y)
    {
        return true;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return true;
    }

    return false;
}

bool SBR2AIBrain::can_use_surrounded_kick_escape_right(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 右隣に爆弾がないならキック対象外
    if (!board.is_bomb(x + 1, y))
    {
        return false;
    }

    // 囲われ判定:
    // 上・左・下が通れないなら、右へ崩したい状況
    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(up_blocked && left_blocked && down_blocked))
    {
        return false;
    }

    // キックは「右隣1個だけ」で、その先が空いているときに使う
    // 2連以上なら今回はパンチ側に任せる
    if (board.is_bomb(x + 2, y))
    {
        return false;
    }

    // 少なくとも1マス先には進める必要がある
    const i8 next_x = x + 2;
    const i8 next_y = y;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    // 2マス間隔の包囲列候補:
    // 爆弾-空き-爆弾 なら普通キックではなく
    // 1マスキックストップ候補として扱いたいので除外する
    const i8 far_x = x + 3;
    const i8 far_y = y;

    if (board.is_inside(far_x, far_y) && board.is_bomb(far_x, far_y))
    {
        return false;
    }

    return true;
}

bool SBR2AIBrain::can_use_surrounded_kick_escape_left(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    if (!board.is_bomb(x - 1, y))
    {
        return false;
    }

    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(up_blocked && right_blocked && down_blocked))
    {
        return false;
    }

    if (board.is_bomb(x - 2, y))
    {
        return false;
    }

    const i8 next_x = x - 2;
    const i8 next_y = y;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    // 2マス間隔の包囲列候補:
    // 爆弾-空き-爆弾 なら普通キックではなく
    // 1マスキックストップ候補として扱いたいので除外する
    const i8 far_x = x - 3;
    const i8 far_y = y;

    if (board.is_inside(far_x, far_y) && board.is_bomb(far_x, far_y))
    {
        return false;
    }

    return true;
}

bool SBR2AIBrain::can_use_surrounded_kick_escape_up(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    if (!board.is_bomb(x, y - 1))
    {
        return false;
    }

    // 2マス間隔の包囲列候補:
    // 爆弾-空き-爆弾 の並びなら、普通キックではなく
    // 1マスキックストップ候補として扱いたいので除外する
    const i8 far_x = x;
    const i8 far_y = y - 3;

    if (board.is_inside(far_x, far_y) && board.is_bomb(far_x, far_y))
    {
        return false;
    }

    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(left_blocked && right_blocked && down_blocked))
    {
        return false;
    }

    if (board.is_bomb(x, y - 2))
    {
        return false;
    }

    const i8 next_x = x;
    const i8 next_y = y - 2;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    return true;
}

bool SBR2AIBrain::can_use_surrounded_kick_escape_down(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    if (!board.is_bomb(x, y + 1))
    {
        return false;
    }

    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    if (!(up_blocked && left_blocked && right_blocked))
    {
        return false;
    }

    if (board.is_bomb(x, y + 2))
    {
        return false;
    }

    const i8 next_x = x;
    const i8 next_y = y + 2;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    return true;
}

bool SBR2AIBrain::can_use_enclosure_kick_stop_up(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 上隣に爆弾が無いなら対象外
    if (!board.is_bomb(x, y - 1))
    {
        return false;
    }

    // 左右下が塞がっていること
    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(left_blocked && right_blocked && down_blocked))
    {
        return false;
    }

    // 1マス先は空き
    const i8 next_x = x;
    const i8 next_y = y - 2;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    // さらにその先に爆弾がある
    // ＝ 爆弾-空き-爆弾 の2マス間隔包囲
    const i8 far_x = x;
    const i8 far_y = y - 3;

    if (!board.is_inside(far_x, far_y))
    {
        return false;
    }

    if (!board.is_bomb(far_x, far_y))
    {
        return false;
    }

    return true;
}

bool SBR2AIBrain::can_use_enclosure_kick_stop_left(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 左隣に爆弾が無いなら対象外
    if (!board.is_bomb(x - 1, y))
    {
        return false;
    }

    // 上右下が塞がっていること
    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(up_blocked && right_blocked && down_blocked))
    {
        return false;
    }

    // 1マス先は空き
    const i8 next_x = x - 2;
    const i8 next_y = y;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    // さらにその先に爆弾がある
    const i8 far_x = x - 3;
    const i8 far_y = y;

    if (!board.is_inside(far_x, far_y))
    {
        return false;
    }

    if (!board.is_bomb(far_x, far_y))
    {
        return false;
    }

    return true;
}

bool SBR2AIBrain::can_use_enclosure_kick_stop_right(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 右隣に爆弾が無いなら対象外
    if (!board.is_bomb(x + 1, y))
    {
        return false;
    }

    // 上左下が塞がっていること
    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool down_blocked =
        !board.is_inside(x, y + 1) ||
        !board.is_passable(x, y + 1);

    if (!(up_blocked && left_blocked && down_blocked))
    {
        return false;
    }

    // 1マス先は空き
    const i8 next_x = x + 2;
    const i8 next_y = y;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    // さらにその先に爆弾がある
    const i8 far_x = x + 3;
    const i8 far_y = y;

    if (!board.is_inside(far_x, far_y))
    {
        return false;
    }

    if (!board.is_bomb(far_x, far_y))
    {
        return false;
    }

    return true;
}

bool SBR2AIBrain::can_use_enclosure_kick_stop_down(i8 x, i8 y, i32 frame) const
{
    (void)frame;

    const SBR2Board &board = simulator_.board();

    if (!board.is_inside(x, y))
    {
        return false;
    }

    // 下隣に爆弾が無いなら対象外
    if (!board.is_bomb(x, y + 1))
    {
        return false;
    }

    // 上左右が塞がっていること
    bool up_blocked =
        !board.is_inside(x, y - 1) ||
        !board.is_passable(x, y - 1);

    bool left_blocked =
        !board.is_inside(x - 1, y) ||
        !board.is_passable(x - 1, y);

    bool right_blocked =
        !board.is_inside(x + 1, y) ||
        !board.is_passable(x + 1, y);

    if (!(up_blocked && left_blocked && right_blocked))
    {
        return false;
    }

    // 1マス先は空き
    const i8 next_x = x;
    const i8 next_y = y + 2;

    if (!board.is_inside(next_x, next_y))
    {
        return false;
    }

    if (!board.is_passable(next_x, next_y))
    {
        return false;
    }

    // さらにその先に爆弾がある
    const i8 far_x = x;
    const i8 far_y = y + 3;

    if (!board.is_inside(far_x, far_y))
    {
        return false;
    }

    if (!board.is_bomb(far_x, far_y))
    {
        return false;
    }

    return true;
}

SBR2Action SBR2AIBrain::decide_next_action(i8 x, i8 y, i32 frame) const
{
    g_last_bomb_reason.clear();
    SBR2EscapeResult result{};

    if (can_use_surrounded_punch_escape_right(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_punch_escape_right";
        return reset_reposition_state_and_return(SBR2Action::PUNCH_RIGHT);
    }

    if (can_use_surrounded_punch_escape_left(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_punch_escape_left";
        return reset_reposition_state_and_return(SBR2Action::PUNCH_LEFT);
    }

    if (can_use_surrounded_punch_escape_up(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_punch_escape_up";
        return reset_reposition_state_and_return(SBR2Action::PUNCH_UP);
    }

    if (can_use_surrounded_punch_escape_down(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_punch_escape_down";
        return reset_reposition_state_and_return(SBR2Action::PUNCH_DOWN);
    }

    if (can_use_surrounded_kick_escape_right(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_kick_escape_right";
        return reset_reposition_state_and_return(SBR2Action::KICK_RIGHT);
    }

    if (can_use_surrounded_kick_escape_left(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_kick_escape_left";
        return reset_reposition_state_and_return(SBR2Action::KICK_LEFT);
    }

    if (can_use_enclosure_kick_stop_left(x, y, frame))
    {
        g_last_bomb_reason = "enclosure_kick_stop_left";
        return reset_reposition_state_and_return(SBR2Action::KICK_STOP_LEFT);
    }

    if (can_use_enclosure_kick_stop_right(x, y, frame))
    {
        g_last_bomb_reason = "enclosure_kick_stop_right";
        return reset_reposition_state_and_return(SBR2Action::KICK_STOP_RIGHT);
    }

    if (can_use_enclosure_kick_stop_up(x, y, frame))
    {
        g_last_bomb_reason = "enclosure_kick_stop_up";
        return reset_reposition_state_and_return(SBR2Action::KICK_STOP_UP);
    }

    if (can_use_surrounded_kick_escape_up(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_kick_escape_up";
        return reset_reposition_state_and_return(SBR2Action::KICK_UP);
    }

    if (can_use_enclosure_kick_stop_down(x, y, frame))
    {
        g_last_bomb_reason = "enclosure_kick_stop_down";
        return reset_reposition_state_and_return(SBR2Action::KICK_STOP_DOWN);
    }

    if (can_use_surrounded_kick_escape_down(x, y, frame))
    {
        g_last_bomb_reason = "surrounded_kick_escape_down";
        return reset_reposition_state_and_return(SBR2Action::KICK_DOWN);
    }

    bool can_escape = pathfinder_.find_escape_action(x, y, frame, result);
    bool dangerous_soon = will_be_dangerous_soon(x, y, frame);
    bool can_punch_escape_right =
        can_use_surrounded_punch_escape_right(x, y, frame);

    if (!can_escape || dangerous_soon)
    {
        if (can_escape)
        {
            return reset_reposition_state_and_return(result.first_action);
        }

        // 囲われ脱出（最小版）
        // まずは右方向パンチだけ対応
        if (can_use_surrounded_punch_escape_right(x, y, frame))
        {
            g_last_bomb_reason = "surrounded_punch_escape_right";
            return reset_reposition_state_and_return(SBR2Action::PUNCH_RIGHT);
        }

        g_last_bomb_reason =
            std::string("escape_fail") + " can_escape=" + (can_escape ? "1" : "0") + " dangerous_soon=" + (dangerous_soon ? "1" : "0") + " punch_right=" + (can_punch_escape_right ? "1" : "0");

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
                g_last_bomb_reason = "trap_first";
                return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
            }
        }

        // ===== 近距離ゴリ押し（対人強化） =====
        if (style() == SBR2AIStyle::Aggressive ||
            (style() == SBR2AIStyle::Tricky && normalized_ai_level() >= 18))
        {
            i8 enemy_x = simulator_.board().enemy_x();
            i8 enemy_y = simulator_.board().enemy_y();

            int enemy_dist = std::abs(enemy_x - x) + std::abs(enemy_y - y);

            if (enemy_dist <= 2)
            {
                // トラップ系を優先させるため条件を少し絞る
                if (enemy_dist == 1)
                {
                    // Trickyは近距離ゴリ押しを使わず、後段のtrap系へ流す
                    if (style() != SBR2AIStyle::Tricky)
                    {
                        int pseudo_rand = (frame + x * 11 + y * 17) % 4;

                        if (pseudo_rand != 0)
                        {
                            if (can_place_bomb_and_escape(x, y, frame))
                            {
                                last_bomb_frame_ = frame;
                                g_last_bomb_reason = "close_range_pressure";
                                return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                            }
                        }
                    }
                }
            }

            // =========================
            // 近距離詰ませチェック（確殺優先）
            // =========================
            {
                i8 enemy_x = simulator_.board().enemy_x();
                i8 enemy_y = simulator_.board().enemy_y();

                int enemy_dist = std::abs(enemy_x - x) + std::abs(enemy_y - y);

                if (enemy_dist == 1)
                {
                    int escape = 0;

                    const int dx[4] = {1, -1, 0, 0};
                    const int dy[4] = {0, 0, 1, -1};

                    for (int i = 0; i < 4; ++i)
                    {
                        int nx = enemy_x + dx[i];
                        int ny = enemy_y + dy[i];

                        if (can_step_to(nx, ny))
                        {
                            escape++;
                        }
                    }

                    // 逃げ道1以下なら確殺扱い
                    if (escape <= 1)
                    {
                        if (can_place_bomb_and_escape(x, y, frame))
                        {
                            last_bomb_frame_ = frame;
                            g_last_bomb_reason = "close_range_checkmate";
                            return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                        }
                    }
                }
            }
        }

        // ===== 完全詰み判定（Tricky最優先） =====
        if (style() == SBR2AIStyle::Tricky)
        {
            i8 ex = simulator_.board().enemy_x();
            i8 ey = simulator_.board().enemy_y();

            int safe_count = 0;

            const int dx[5] = {0, 1, -1, 0, 0};
            const int dy[5] = {0, 0, 0, 1, -1};

            for (int i = 0; i < 5; ++i)
            {
                i8 nx = ex + dx[i];
                i8 ny = ey + dy[i];

                if (!can_step_to(nx, ny))
                    continue;

                // そのマスが安全か軽くチェック
                if (!simulator_.is_danger(frame + 20, nx, ny))
                {
                    safe_count++;
                }
            }

            // 逃げ道がない＝確殺
            if (safe_count == 0)
            {
                if (can_place_bomb_and_escape(x, y, frame))
                {
                    last_bomb_frame_ = frame;
                    g_last_bomb_reason = "checkmate";
                    return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                }
            }
        }

        // ===== 誘導トラップ（Tricky専用・強化版） =====
        if (style() == SBR2AIStyle::Tricky && normalized_ai_level() >= 10)
        {
            i8 ex = simulator_.board().enemy_x();
            i8 ey = simulator_.board().enemy_y();

            int dx = ex - x;
            int dy = ey - y;

            // 同じ行または列
            if (dx == 0 || dy == 0)
            {
                int dist = std::abs(dx) + std::abs(dy);

                if (dist >= 1 && dist <= 6)
                {
                    // 逃げ方向（1マス目）
                    int dir_x = (dx != 0) ? (dx > 0 ? 1 : -1) : 0;
                    int dir_y = (dy != 0) ? (dy > 0 ? 1 : -1) : 0;

                    i8 e1x = ex + dir_x;
                    i8 e1y = ey + dir_y;

                    // 2マス目
                    i8 e2x = e1x + dir_x;
                    i8 e2y = e1y + dir_y;

                    bool escape1_ok = can_step_to(e1x, e1y);
                    bool escape2_ok = can_step_to(e2x, e2y);

                    bool escape1_danger = false;
                    bool escape2_danger = false;

                    if (escape1_ok)
                    {
                        escape1_danger = simulator_.is_danger(frame + 18, e1x, e1y);
                    }

                    if (escape2_ok)
                    {
                        escape2_danger = simulator_.is_danger(frame + 25, e2x, e2y);
                    }

                    // 1マス目が塞がっている、または
                    // 1マス目/2マス目が危険、または2マス目が塞がっていれば
                    // 誘導トラップ成立寄りとみなす
                    bool trap_like =
                        !escape1_ok ||
                        escape1_danger ||
                        !escape2_ok ||
                        escape2_danger ||
                        (escape1_ok && escape2_ok);

                    if (trap_like)
                    {
                        if (can_place_bomb_and_escape(x, y, frame))
                        {
                            last_bomb_frame_ = frame;
                            g_last_bomb_reason = "guided_trap_v2";
                            return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                        }
                    }
                }
            }
        }

        // 直線キル判定
        if (level_allows_straight_kill())
        {


            if (can_hit_enemy_now_or_after_one_step(
                    x, y, level_allows_one_step_prediction()))
            {
                bool safe = can_place_bomb_and_escape(x, y, frame);

                if (style() == SBR2AIStyle::Tricky)
                {
                    safe = can_place_bomb_and_escape_strict(x, y, frame);
                }

                // 近距離なら少し強気にする
                i8 enemy_x = simulator_.board().enemy_x();
                i8 enemy_y = simulator_.board().enemy_y();
                int enemy_dist = std::abs(enemy_x - x) + std::abs(enemy_y - y);

                if (enemy_dist <= 2)
                {
                    // 多少リスクがあっても置きに行く
                    safe = safe || can_place_bomb_and_escape(x, y, frame);
                }

                if (style() == SBR2AIStyle::Careful)
                {
                    safe = can_place_bomb_and_escape_strict(x, y, frame);
                }

                bool allow_straight_kill = (safe || allow_risky);

                if (style() == SBR2AIStyle::Careful && enemy_dist >= 4)
                {
                    allow_straight_kill = false;
                }

                // Careful は近距離でも少しだけ攻めを抑える
                if (style() == SBR2AIStyle::Careful && enemy_dist <= 2)
                {
                    allow_straight_kill = false;
                }

                // Tricky は誘導トラップや誘導ボムを優先しやすくするため、
                // 斜め近距離では直線キルを少し抑える
                if (style() == SBR2AIStyle::Tricky)
                {
                    allow_straight_kill = safe;

                    bool diagonal_relation = (enemy_x != x && enemy_y != y);
                    if (diagonal_relation && enemy_dist <= 2)
                    {
                        allow_straight_kill = false;
                    }
                }

                if (allow_straight_kill)
                {
                    last_bomb_frame_ = frame;
                    g_last_bomb_reason = "straight_kill";
                    return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                }
            }


            // ===== 誘導ボム（簡易） =====
            if (style() == SBR2AIStyle::Tricky ||
                (style() == SBR2AIStyle::Aggressive && normalized_ai_level() >= 15))
            {
                i8 enemy_x = simulator_.board().enemy_x();
                i8 enemy_y = simulator_.board().enemy_y();

                int dx = enemy_x - x;
                int dy = enemy_y - y;

                if (dx != 0 && dy != 0 && std::abs(dx) + std::abs(dy) <= 4)
                {
                    bool enemy_can_escape_lr =
                        can_step_to(enemy_x - 1, enemy_y) ||
                        can_step_to(enemy_x + 1, enemy_y);

                    bool enemy_can_escape_ud =
                        can_step_to(enemy_x, enemy_y - 1) ||
                        can_step_to(enemy_x, enemy_y + 1);

                    if (enemy_can_escape_lr && std::abs(dx) <= 2)
                    {
                        if (can_place_bomb_and_escape(x, y, frame))
                        {
                            last_bomb_frame_ = frame;
                            g_last_bomb_reason = "guided_bomb";
                            return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                        }
                    }

                    if (enemy_can_escape_ud && std::abs(dy) <= 2)
                    {
                        if (can_place_bomb_and_escape(x, y, frame))
                        {
                            last_bomb_frame_ = frame;
                            g_last_bomb_reason = "guided_bomb";
                            return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                        }
                    }
                }
            }

            // =========================
            // 逃げ方向誘導トラップ（上級）
            // =========================
            if (style() == SBR2AIStyle::Tricky ||
                (style() == SBR2AIStyle::Aggressive && normalized_ai_level() >= 18))
            {
                i8 enemy_x = simulator_.board().enemy_x();
                i8 enemy_y = simulator_.board().enemy_y();

                int dx = enemy_x - x;
                int dy = enemy_y - y;
                int enemy_dist = std::abs(dx) + std::abs(dy);

                if (enemy_dist >= 2 && enemy_dist <= 5)
                {
                    int dir_x = 0;
                    int dir_y = 0;

                    if (std::abs(dx) > std::abs(dy))
                    {
                        dir_x = (dx > 0 ? 1 : -1);
                    }
                    else
                    {
                        dir_y = (dy > 0 ? 1 : -1);
                    }

                    int escape_x = enemy_x + dir_x;
                    int escape_y = enemy_y + dir_y;

                    if (can_step_to(escape_x, escape_y))
                    {
                        bool guided_line =
                            can_hit_enemy_in_straight_line(
                                x, y,
                                static_cast<i8>(escape_x),
                                static_cast<i8>(escape_y));

                        if (guided_line)
                        {
                            int survivable =
                                count_enemy_survivable_cells_after_bomb(x, y, frame);

                            if (survivable <= 3)
                            {
                                if (can_place_bomb_and_escape(x, y, frame))
                                {
                                    last_bomb_frame_ = frame;
                                    g_last_bomb_reason = "guided_trap";
                                    return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                                }
                            }
                        }
                    }
                }
            }

            // ===== 詰ませボム（強化版＋距離依存） =====
            if (style() == SBR2AIStyle::Tricky ||
                (style() == SBR2AIStyle::Aggressive && normalized_ai_level() >= 17))
            {
                i8 enemy_x = simulator_.board().enemy_x();
                i8 enemy_y = simulator_.board().enemy_y();

                int enemy_dist = std::abs(enemy_x - x) + std::abs(enemy_y - y);

                int survivable =
                    count_enemy_survivable_cells_after_bomb(x, y, frame);

                int threshold = 2;

                // 🔥 距離が近いほど強気にする
                if (enemy_dist <= 2)
                {
                    threshold = 3; // かなり攻める
                }
                else if (enemy_dist <= 4)
                {
                    threshold = 2;
                }
                else
                {
                    threshold = 1; // 遠いときは慎重
                }

                // Trickyはさらに攻める
                if (style() == SBR2AIStyle::Tricky && normalized_ai_level() >= 18)
                {
                    threshold += 1;
                }

                if (survivable <= threshold)
                {
                    if (can_place_bomb_and_escape(x, y, frame))
                    {
                        last_bomb_frame_ = frame;
                        g_last_bomb_reason = "survivable_checkmate";
                        return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
                    }
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
                g_last_bomb_reason = "trap_normal";
                return reset_reposition_state_and_return(SBR2Action::PLACE_BOMB);
            }
        }

        // 高レベルTrickyだけ、最後にもう一度 trap を見る
        if (tricky_trap_second_chance && level_allows_trap())
        {
            if (is_trap_possible(x, y, frame))
            {
                last_bomb_frame_ = frame;
                g_last_bomb_reason = "trap_second_chance";
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

        int reposition_start_dist = 4;

        if (style() == SBR2AIStyle::Aggressive && normalized_ai_level() >= 15)
        {
            reposition_start_dist = 3;
        }
        else if (style() == SBR2AIStyle::Tricky && normalized_ai_level() >= 15)
        {
            reposition_start_dist = 3;
        }

        if (dist >= reposition_start_dist)
        {
            if (settings_.style == SBR2AIStyle::Aggressive)
            {
                return remember_reposition_action(
                    apply_reposition_hold(move_toward_enemy(x, y, frame), x, y, frame));
            }

            if (settings_.style == SBR2AIStyle::Tricky)
            {
                if (enemy_x != x && enemy_y != y)
                {
                    return remember_reposition_action(
                        apply_reposition_hold(move_toward_enemy(x, y, frame), x, y, frame));
                }

                if (dist >= 2)
                {
                    return remember_reposition_action(
                        apply_reposition_hold(move_toward_enemy(x, y, frame), x, y, frame));
                }
            }

            if (settings_.style == SBR2AIStyle::Careful)
            {
                if (dist >= 6)
                {
                    return remember_reposition_action(
                        apply_reposition_hold(move_toward_enemy(x, y, frame), x, y, frame));
                }
            }
        }
    }

    // 安全なら軽く詰める（WAIT減らす）
    // if (!will_be_dangerous_soon(x, y, frame))
    // {
    //     SBR2Action move = move_toward_enemy(x, y, frame);
    //     return remember_reposition_action(
    //         apply_reposition_hold(move, x, y, frame));
    // }

    return reset_reposition_state_and_return(SBR2Action::WAIT);
}

SBR2Action SBR2AIBrain::decide_reposition_action_for_test(
    i8 x, i8 y, i32 frame) const
{
    return remember_reposition_action(
        apply_reposition_hold(move_toward_enemy(x, y, frame), x, y, frame));
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

    bool can_left = can_step_to(enemy_x - 1, enemy_y);
    bool can_right = can_step_to(enemy_x + 1, enemy_y);
    bool can_up = can_step_to(enemy_x, enemy_y - 1);
    bool can_down = can_step_to(enemy_x, enemy_y + 1);

    SBR2Action first = SBR2Action::WAIT;
    SBR2Action second = SBR2Action::WAIT;

    bool diagonal_relation = (dx != 0 && dy != 0);
    int enemy_dist = std::abs(dx) + std::abs(dy);

    // 基本は遠い軸を優先
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

    // 横に逃げやすいなら横方向を優先
    if ((can_left || can_right) && std::abs(dx) <= 3)
    {
        if (dx > 0)
            first = SBR2Action::RIGHT;
        else if (dx < 0)
            first = SBR2Action::LEFT;
    }

    // 縦に逃げやすいなら縦方向を優先
    // ただし、横方向の差のほうが大きいときは上書きしない
    if ((can_up || can_down) && std::abs(dy) <= 3 && std::abs(dy) > std::abs(dx))
    {
        if (dy > 0)
            first = SBR2Action::DOWN;
        else if (dy < 0)
            first = SBR2Action::UP;
    }

    // 斜め + 近距離では、行/列を合わせる動きを少し優先する
    // Tricky や高レベル Aggressive が、罠の形を作りやすくする狙い
    if (diagonal_relation && enemy_dist <= 5)
    {
        bool prefer_line_up =
            (style() == SBR2AIStyle::Tricky) ||
            (style() == SBR2AIStyle::Aggressive && normalized_ai_level() >= 15);

        if (prefer_line_up)
        {
            // 差が小さいときは frame に応じて優先軸を少し揺らす
            // ただし「近づく方向」自体は維持する
            if (std::abs(dx) == std::abs(dy))
            {
                if ((frame % 2) == 0)
                {
                    std::swap(first, second);
                }
            }
            else if (std::abs(dx) + 1 >= std::abs(dy) &&
                     std::abs(dy) + 1 >= std::abs(dx))
            {
                if ((frame % 3) == 1)
                {
                    std::swap(first, second);
                }
            }
        }
    }

    // 敵が近いとき、たまに第2候補を優先して動きに揺れを出す
    // ただし一直線で詰められる形なら、揺れを抑えて素直に詰める
    bool aligned_straight = (dx == 0 || dy == 0);

    // ===== 人間っぽい揺れ（疑似ランダム） =====
    // 完全ランダムではなく frame を使って再現性を保つ
    if (enemy_dist <= 5 && !aligned_straight)
    {
        int pseudo_rand = (frame + self_x * 7 + self_y * 13) % 5;

        // 近距離では揺れを弱める
        if (enemy_dist >= 4 && pseudo_rand == 0)
        {
            std::swap(first, second);
        }
    }

    if (enemy_dist <= 4 && !aligned_straight)
    {
        // かなり近いときは往復しやすいので、frame依存の揺れを止める
        if (enemy_dist >= 4 && (frame % 3) == 0)
        {
            std::swap(first, second);
        }
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

    bool avoid_same_direction_too_much =
        is_move_action(first) &&
        first == last_reposition_action_ &&
        same_direction_reposition_count_ >= same_direction_reposition_limit(self_x, self_y);

    if (avoid_same_direction_too_much && can_use_action(second))
    {
        return second;
    }

    if (can_use_action(first))
    {
        // 直前と逆方向への往復を少し抑える
        bool reverse_of_last =
            (first == SBR2Action::UP && last_reposition_action_ == SBR2Action::DOWN) ||
            (first == SBR2Action::DOWN && last_reposition_action_ == SBR2Action::UP) ||
            (first == SBR2Action::LEFT && last_reposition_action_ == SBR2Action::RIGHT) ||
            (first == SBR2Action::RIGHT && last_reposition_action_ == SBR2Action::LEFT);

        // すでに敵と同じ列にいて、さらに縦へ行くと行き過ぎやすいなら少し抑える
        bool overshoot_vertical =
            (self_x == enemy_x) &&
            ((first == SBR2Action::UP && self_y < enemy_y) ||
             (first == SBR2Action::DOWN && self_y > enemy_y));

        // すでに敵と同じ行にいて、さらに横へ行くと行き過ぎやすいなら少し抑える
        bool overshoot_horizontal =
            (self_y == enemy_y) &&
            ((first == SBR2Action::LEFT && self_x < enemy_x) ||
             (first == SBR2Action::RIGHT && self_x > enemy_x));

        if ((!reverse_of_last && !overshoot_vertical && !overshoot_horizontal) ||
            !can_use_action(second))
        {
            return first;
        }
    }

    if (can_use_action(second))
    {
        return second;
    }

    // ここまでで first / second が使えないなら、
    // 「通れるだけ」ではなく「敵に近づく fallback」を選ぶ
    SBR2Action best_fallback = SBR2Action::WAIT;
    int best_dist = 9999;

    auto try_fallback = [&](SBR2Action action, i8 nx, i8 ny)
    {
        if (!can_step_to(nx, ny))
        {
            return;
        }

        int dist_after = std::abs(enemy_x - nx) + std::abs(enemy_y - ny);

        bool better = false;

        if (dist_after < best_dist)
        {
            better = true;
        }
        else if (dist_after == best_dist)
        {
            // 同点なら、直前と同じ方向を優先して往復を減らす
            if (action == last_reposition_action_ &&
                best_fallback != last_reposition_action_)
            {
                better = true;
            }
        }

        if (better)
        {
            best_dist = dist_after;
            best_fallback = action;
        }
    };

    try_fallback(SBR2Action::UP, self_x, self_y - 1);
    try_fallback(SBR2Action::DOWN, self_x, self_y + 1);
    try_fallback(SBR2Action::LEFT, self_x - 1, self_y);
    try_fallback(SBR2Action::RIGHT, self_x + 1, self_y);

    if (best_fallback != SBR2Action::WAIT)
    {
        return best_fallback;
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