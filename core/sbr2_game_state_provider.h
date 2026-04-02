#pragma once

#include "core/sbr2_game_state.h"

class SBR2GameStateProvider
{
public:
    virtual ~SBR2GameStateProvider() = default;

    virtual SBR2GameState get_state(int tick) = 0;
};