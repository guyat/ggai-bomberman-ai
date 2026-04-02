#pragma once

#include "core/sbr2_game_state_provider.h"

class SBR2VisionGameStateProvider : public SBR2GameStateProvider
{
public:
    SBR2GameState get_state(int tick) override;
};