#include "core/sbr2_dummy_game_state_provider.h"

namespace
{
    constexpr int kReadyFrames = 50;
    constexpr int kGoFrames = 100;
    constexpr int kResultFrames = 30;

    constexpr int kGoStartFrame = kReadyFrames;
    constexpr int kResultStartFrame = kGoStartFrame + kGoFrames;
    constexpr int kRoundFrames = kResultStartFrame + kResultFrames;
    constexpr int kRoundCount = 4;
}

SBR2GameState SBR2DummyGameStateProvider::get_state(int tick)
{
    SBR2GameState s;

    int round_index = (tick / kRoundFrames) % kRoundCount;
    int round_tick = tick % kRoundFrames;

    s.self_found = true;

    switch (round_index)
    {
    case 0: // 左上
        s.self_x = 0;
        s.self_y = 0;
        break;
    case 1: // 右上
        s.self_x = 12;
        s.self_y = 0;
        break;
    case 2: // 左下
        s.self_x = 0;
        s.self_y = 10;
        break;
    default: // 右下
        s.self_x = 12;
        s.self_y = 10;
        break;
    }

    s.enemy_found = true;
    s.enemy_x = 6;
    s.enemy_y = 5;

    if (round_tick < kReadyFrames)
    {
        s.phase = SBR2Phase::READY;
    }
    else if (round_tick < kResultStartFrame)
    {
        s.phase = SBR2Phase::GO;
    }
    else
    {
        s.phase = SBR2Phase::RESULT;
    }

    return s;
}