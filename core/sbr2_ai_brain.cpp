#include "sbr2_ai_brain.h"
#include <cmath>

static int last_bomb_frame = -1000;

SBR2AIBrain::SBR2AIBrain(
    const SBR2Simulator& simulator,
    const SBR2PathFinder& pathfinder
)
    : simulator_(simulator),
      pathfinder_(pathfinder)
{
}

bool SBR2AIBrain::will_be_dangerous_soon(i8 x, i8 y, i32 frame) const
{
    const int LOOKAHEAD = 40;

    for (int t = 0; t <= LOOKAHEAD; ++t)
    {
        if (simulator_.is_danger(frame + t, x, y))
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

    // すでに危険マスなら置かない
    if (simulator_.is_danger(frame, x, y))
    {
        return false;
    }

    // すでにそのマスに爆弾があるなら置かない
    if (simulator_.board().is_bomb(x, y))
    {
        return false;
    }

    // 爆弾を置いた未来を別シミュレーション
    SBR2Simulator sim_after_place = simulator_;
    sim_after_place.add_bomb(x, y, frame);
    sim_after_place.simulate();

    SBR2PathFinder pathfinder_after(
        sim_after_place.board(),
        sim_after_place
    );

    SBR2EscapeResult result{};

    // 自分で置いた爆弾の爆発～爆風終了まで
    const i32 safe_until_frame =
        frame + SBR2Bomb::EXPLOSION_TIMER + SBR2Bomb::FIRE_DURATION;

    return pathfinder_after.find_escape_action_until(
        x,
        y,
        frame,
        safe_until_frame,
        result
    );
}

bool SBR2AIBrain::is_enemy_close(i8 x, i8 y) const
{
    i8 ex = simulator_.board().enemy_x();
    i8 ey = simulator_.board().enemy_y();

    if (ex < 0 || ey < 0)
    {
        return false;
    }

    int dist = std::abs(ex - x) + std::abs(ey - y);

    return dist <= 3;
}

SBR2Action SBR2AIBrain::decide_next_action(i8 x, i8 y, i32 frame) const
{
    SBR2EscapeResult result{};

    // =========================
    // ① 逃げられるかチェック
    // =========================
    bool can_escape = pathfinder_.find_escape_action(x, y, frame, result);

    // 逃げられない or 危険になるなら逃げる
    if (!can_escape || will_be_dangerous_soon(x, y, frame))
    {
        if (can_escape)
        {
            return result.first_action;
        }

        return SBR2Action::WAIT;
    }

    // =========================
    // ② 攻撃判断
    // =========================
    if (is_enemy_close(x, y))
    {
        if (can_place_bomb_and_escape(x, y, frame))
        {
            if (frame - last_bomb_frame > 20)
            {
                last_bomb_frame = frame;
                return SBR2Action::PLACE_BOMB;
            }
        }
    }

    // =========================
    // ③ 待機
    // =========================
    return SBR2Action::WAIT;
}