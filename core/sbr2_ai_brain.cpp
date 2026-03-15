#include "sbr2_ai_brain.h"

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
    const int LOOKAHEAD = 20;

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

    if (simulator_.is_danger(frame, x, y))
    {
        return false;
    }

    // 置いた後の未来を丸ごと別シミュレーションする
    SBR2Simulator sim_after_place = simulator_;
    sim_after_place.add_bomb(x, y, frame);
    sim_after_place.simulate();

    SBR2PathFinder pathfinder_after(
        sim_after_place.board(),
        sim_after_place
    );

    SBR2EscapeResult result{};

    // 自分で置いた爆弾の爆発終了まで生き残れるか確認
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

SBR2Action SBR2AIBrain::decide_next_action(i8 x, i8 y, i32 frame) const
{
    // 近い未来に危険化するなら、まず逃げる
    if (will_be_dangerous_soon(x, y, frame))
    {
        SBR2EscapeResult result{};

        if (pathfinder_.find_escape_action(x, y, frame, result))
        {
            return result.first_action;
        }

        return SBR2Action::WAIT;
    }

    // 安全なら、爆弾を置いてから逃げ切れるか試す
    if (can_place_bomb_and_escape(x, y, frame))
    {
        return SBR2Action::PLACE_BOMB;
    }

    return SBR2Action::WAIT;
}