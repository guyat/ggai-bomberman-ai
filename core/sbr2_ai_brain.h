#pragma once

#include "sbr2_pathfinder.h"
#include "sbr2_simulator.h"

class SBR2AIBrain
{
public:

    SBR2AIBrain(
        const SBR2Simulator& simulator,
        const SBR2Pathfinder& pathfinder
    );

    SBR2Action decide_next_action(
        int x,
        int y,
        int frame
    ) const;

private:

    const SBR2Simulator& simulator_;
    const SBR2Pathfinder& pathfinder_;

};