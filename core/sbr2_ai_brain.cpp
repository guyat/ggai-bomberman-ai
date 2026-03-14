#include "sbr2_ai_brain.h"

SBR2AIBrain::SBR2AIBrain(
    const SBR2Simulator& simulator,
    const SBR2Pathfinder& pathfinder
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

    // 現在位置が危険か確認
    if (simulator_.is_danger(x, y, frame))
    {
        SBR2EscapeResult result =
            pathfinder_.find_escape_action(x, y, frame);

        if (result.found)
        {
            return result.first_action;
        }
    }

    // 危険じゃない場合
    return SBR2Action::WAIT;
}