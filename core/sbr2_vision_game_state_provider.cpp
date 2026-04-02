#include "core/sbr2_vision_game_state_provider.h"

#include <iostream>
#include <utility>
#include "core/sbr2_screen_capture.h"

namespace
{
    constexpr bool kUseDummyVisionPhase = true;
    constexpr bool kUseDummyVisionSelfPosition = true;
    constexpr bool kUseDummyVisionEnemyPosition = true;

    constexpr int kReadyFrames = 50;
    constexpr int kGoFrames = 100;
    constexpr int kResultFrames = 30;

    constexpr int kGoStartFrame = kReadyFrames;
    constexpr int kResultStartFrame = kGoStartFrame + kGoFrames;
    constexpr int kRoundFrames = kResultStartFrame + kResultFrames;
    constexpr int kRoundCount = 4;

    bool is_ready_visible(int tick)
    {
        if (!kUseDummyVisionPhase)
        {
            return false;
        }

        int round_tick = tick % kRoundFrames;
        return round_tick < kReadyFrames;
    }

    bool is_result_visible(int tick)
    {
        if (!kUseDummyVisionPhase)
        {
            return false;
        }

        int round_tick = tick % kRoundFrames;
        return round_tick >= kResultStartFrame;
    }

    SBR2Phase detect_phase(int tick)
    {
        if (is_ready_visible(tick))
        {
            return SBR2Phase::READY;
        }

        if (is_result_visible(tick))
        {
            return SBR2Phase::RESULT;
        }

        return SBR2Phase::GO;
    }

    std::pair<int, int> detect_self_position_from_frame(int tick)
    {
        if (!kUseDummyVisionSelfPosition)
        {
            return {-1, -1};
        }

        int round_index = (tick / kRoundFrames) % kRoundCount;

        switch (round_index)
        {
        case 0:
            return {0, 0};
        case 1:
            return {12, 0};
        case 2:
            return {0, 10};
        default:
            return {12, 10};
        }
    }

    std::pair<int, int> detect_enemy_position_from_frame(int tick)
    {
        if (!kUseDummyVisionEnemyPosition)
        {
            return {-1, -1};
        }

        (void)tick;
        return {6, 5};
    }

    std::pair<int, int> detect_self_position(int tick)
    {
        return detect_self_position_from_frame(tick);
    }

    std::pair<int, int> detect_enemy_position(int tick)
    {
        return detect_enemy_position_from_frame(tick);
    }
}

SBR2GameState SBR2VisionGameStateProvider::get_state(int tick)
{
    if (tick % 30 == 0)
    {
        std::cout << "[vision] get_state tick=" << tick << std::endl;
    }

    SBR2GameState s;
    SBR2Image image;
    capture_screen(image);

    auto self_pos = detect_self_position(tick);
    auto enemy_pos = detect_enemy_position(tick);

    s.self_found = (self_pos.first >= 0 && self_pos.second >= 0);
    s.self_x = self_pos.first;
    s.self_y = self_pos.second;

    s.enemy_found = (enemy_pos.first >= 0 && enemy_pos.second >= 0);
    s.enemy_x = enemy_pos.first;
    s.enemy_y = enemy_pos.second;

    s.phase = detect_phase(tick);

    return s;
}