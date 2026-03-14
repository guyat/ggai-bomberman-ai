#include "sbr2_ai_brain.h"

SBR2AIBrain::SBR2AIBrain(
    const SBR2Simulator& simulator,
    const SBR2PathFinder& pathfinder
)
    : simulator_(simulator)
    , pathfinder_(pathfinder)
{
}

SBR2Action SBR2AIBrain::decide_next_action(
    int x,
    int y,
    int frame
) const
{
    // 未来危険チェック
    bool future_danger = false;

    for (int t = 0; t <= 30; ++t)
    {
        int check_frame = frame + t;

        if (check_frame >= SBR2Simulator::MAX_SIMULATION_FRAMES)
        {
            break;
        }

        if (simulator_.is_danger(check_frame, x, y))
        {
            future_danger = true;
            break;
        }
    }

    // 今危険 or 近い未来に危険なら逃げる
    if (future_danger)
    {
        SBR2EscapeResult result{};

        if (pathfinder_.find_escape_action(x, y, frame, result))
        {
            if (result.found)
            {
                return result.first_action;
            }
        }
    }

    // まだ危険でなければ待機
    return SBR2Action::WAIT;
}