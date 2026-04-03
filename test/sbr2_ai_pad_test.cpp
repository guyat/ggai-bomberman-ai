#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>

#include "core/sbr2_ai_brain.h"
#include "core/sbr2_action_sender.h"
#include "core/sbr2_pathfinder.h"
#include "core/sbr2_simulator.h"
#include "core/sbr2_virtual_pad.h"

#include "core/sbr2_game_state.h"
#include <cstdint>
#include "core/sbr2_dummy_game_state_provider.h"
#include "core/sbr2_game_state_provider.h"
#include "core/sbr2_vision_game_state_provider.h"

#include "core/sbr2_screen_capture.h"

#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#endif

namespace
{
    SBR2Action decide_escape_demo_action();
    SBR2Action decide_bomb_demo_action();

    constexpr int kReadyFrames = 50; // ← あとでここだけ変えればOK
    constexpr int kGoFrames = 140;
    constexpr int kResultFrames = 30;

    constexpr int kGoStartFrame = kReadyFrames;
    constexpr int kResultStartFrame = kGoStartFrame + kGoFrames;
    constexpr int kRoundFrames = kResultStartFrame + kResultFrames;
    constexpr int kRoundCount = 4;

    constexpr int kImmediateEscapeFramesAfterBomb = 8;
    constexpr int kPostBombEscapeFrames = 24;
    constexpr int kBombDecisionFrame = 70;

    const char *action_to_string(SBR2Action action)
    {
        switch (action)
        {
        case SBR2Action::WAIT:
            return "WAIT";
        case SBR2Action::RIGHT:
            return "RIGHT";
        case SBR2Action::LEFT:
            return "LEFT";
        case SBR2Action::UP:
            return "UP";
        case SBR2Action::DOWN:
            return "DOWN";
        case SBR2Action::PLACE_BOMB:
            return "PLACE_BOMB";
        case SBR2Action::PUNCH_RIGHT:
            return "PUNCH_RIGHT";
        case SBR2Action::PUNCH_LEFT:
            return "PUNCH_LEFT";
        case SBR2Action::PUNCH_UP:
            return "PUNCH_UP";
        case SBR2Action::PUNCH_DOWN:
            return "PUNCH_DOWN";
        case SBR2Action::KICK_RIGHT:
            return "KICK_RIGHT";
        case SBR2Action::KICK_LEFT:
            return "KICK_LEFT";
        case SBR2Action::KICK_UP:
            return "KICK_UP";
        case SBR2Action::KICK_DOWN:
            return "KICK_DOWN";
        default:
            return "OTHER";
        }
    }

    const char *phase_to_string(SBR2Phase p)
    {
        switch (p)
        {
        case SBR2Phase::READY:
            return "READY";
        case SBR2Phase::GO:
            return "GO";
        case SBR2Phase::RESULT:
            return "RESULT";
        default:
            return "UNKNOWN";
        }
    }

    SBR2Action decide_ready_opening_action(const SBR2GameState &state, int tick)
    {
        if (!state.self_found)
        {
            return SBR2Action::WAIT;
        }

        const int center_x = 6;
        const int center_y = 5;

        int dx = center_x - state.self_x;
        int dy = center_y - state.self_y;

        // READY開幕60フレームだけ中央へ寄る
        if (tick >= 60)
        {
            return SBR2Action::WAIT;
        }

        bool need_horizontal = (dx != 0);
        bool need_vertical = (dy != 0);

        if (need_horizontal && need_vertical)
        {
            // 横→縦→横→縦 で交互に出して、斜め寄りの動きにする
            if ((tick % 2) == 0)
            {
                return (dx > 0) ? SBR2Action::RIGHT : SBR2Action::LEFT;
            }
            else
            {
                return (dy > 0) ? SBR2Action::DOWN : SBR2Action::UP;
            }
        }

        if (need_horizontal)
        {
            return (dx > 0) ? SBR2Action::RIGHT : SBR2Action::LEFT;
        }

        if (need_vertical)
        {
            return (dy > 0) ? SBR2Action::DOWN : SBR2Action::UP;
        }

        return SBR2Action::WAIT;
    }

    std::uint16_t decide_ready_opening_buttons(const SBR2GameState &state)
    {
        if (!state.self_found)
        {
            return 0;
        }

        const int center_x = 6;
        const int center_y = 5;

        int dx = center_x - state.self_x;
        int dy = center_y - state.self_y;

        bool need_left = (dx < 0);
        bool need_right = (dx > 0);
        bool need_up = (dy < 0);
        bool need_down = (dy > 0);

        // ボタンビット:
        // UP=1<<0, DOWN=1<<1, LEFT=1<<2, RIGHT=1<<3
        std::uint16_t buttons = 0;

        if (need_up)
        {
            buttons |= (1u << 0);
        }
        if (need_down)
        {
            buttons |= (1u << 1);
        }
        if (need_left)
        {
            buttons |= (1u << 2);
        }
        if (need_right)
        {
            buttons |= (1u << 3);
        }

        return buttons;
    }

    SBR2Action decide_ready_action(const SBR2GameState &state)
    {
        (void)state;
        return SBR2Action::WAIT;
    }

    SBR2Action decide_go_action(int go_tick)
    {
        if (go_tick < kBombDecisionFrame)
        {
            return decide_escape_demo_action();
        }
        else if (go_tick <= kBombDecisionFrame + kImmediateEscapeFramesAfterBomb)
        {
            return SBR2Action::UP;
        }
        else
        {
            return decide_escape_demo_action();
        }
    }

    SBR2Action decide_result_action()
    {
        return SBR2Action::WAIT;
    }

    // CASE 2 相当: 危険が近いので escape 系 action を返すはず
    SBR2Action decide_escape_demo_action()
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.add_bomb(11, 10, 0);
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(1, 0);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrainSettings settings;
        settings.ai_level = 20;

        SBR2AIBrain brain(simulator, pathfinder, settings);
        return brain.decide_next_action(12, 10, 138);
    }

    // CASE 5 相当: 一直線キルで PLACE_BOMB を返すはず
    SBR2Action decide_bomb_demo_action()
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(8, 10);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrainSettings settings;
        settings.ai_level = 20;

        SBR2AIBrain brain(simulator, pathfinder, settings);
        return brain.decide_next_action(12, 10, 200);
    }

} // namespace

int main()
{
    SBR2VirtualPad pad;

    std::cout << "Press Enter to connect..." << std::endl;
    std::cin.get();

    std::cout << "connecting..." << std::endl;

    if (!pad.connect(1))
    {
        std::cout << "connect failed" << std::endl;
        std::cout << "last_error: " << pad.last_error() << std::endl;
        return 1;
    }

    std::cout << "connect success" << std::endl;
    std::cout << "Press Enter to stop and disconnect..." << std::endl;

    SBR2ActionSender sender(pad);
    std::atomic<bool> running{true};

    std::thread worker([&]()
                       {
        int tick = 0;
        SBR2Action current_action = SBR2Action::WAIT;
        SBR2Action last_printed_action = SBR2Action::WAIT;
        std::uint16_t ready_buttons = 0;
        bool saved_debug_capture = false;
        bool last_f8_down = false;
        int debug_capture_index = 0;

        const int THINK_INTERVAL = 6;

        SBR2Phase last_phase = SBR2Phase::UNKNOWN;
        bool bomb_placed_in_current_go = false;
        bool result_seen_once = false;

        // 今は false のまま Dummy を使う。
        // Vision 実験を始めるときに true に切り替える。
        constexpr bool kUseVisionProvider = true;

        SBR2DummyGameStateProvider dummy_provider;
        SBR2VisionGameStateProvider vision_provider;

        SBR2GameStateProvider *provider =
            kUseVisionProvider
                ? static_cast<SBR2GameStateProvider *>(&vision_provider)
                : static_cast<SBR2GameStateProvider *>(&dummy_provider);

        while (running.load()) {
            SBR2GameState state = provider->get_state(tick);

#ifdef _WIN32
            bool f8_down = (GetAsyncKeyState(VK_F8) & 0x8000) != 0;
            if (f8_down && !last_f8_down)
            {
                SBR2Image debug_image;
                if (capture_screen(debug_image))
                {
                    ++debug_capture_index;

                    char path[64];
                    std::snprintf(path, sizeof(path),
                                  "debug_capture_%03d.bmp",
                                  debug_capture_index);

                    save_image_as_bmp(debug_image, path);
                }
            }
            last_f8_down = f8_down;
#endif

            if (tick % THINK_INTERVAL == 0 || state.phase != last_phase || current_action == SBR2Action::PLACE_BOMB)
            {
                // ===== phaseベース制御 =====
                if (state.phase == SBR2Phase::RESULT)
                {
                    result_seen_once = true;
                }

                if (!result_seen_once && state.phase != SBR2Phase::RESULT)
                {
                    bomb_placed_in_current_go = false;
                    ready_buttons = 0;
                    current_action = SBR2Action::WAIT;
                }
                else if (state.phase == SBR2Phase::READY)
                {
                    bomb_placed_in_current_go = false;
                    ready_buttons = decide_ready_opening_buttons(state);
                    current_action = decide_ready_action(state);
                }
                else if (state.phase == SBR2Phase::GO)
                {
                    ready_buttons = 0;

                    int go_tick = (tick % kRoundFrames) - kGoStartFrame;

                    if (!bomb_placed_in_current_go &&
                        state.phase == SBR2Phase::GO &&
                        !state.go_open_delay_active)
                    {
                        current_action = decide_bomb_demo_action();
                        bomb_placed_in_current_go = true;
                    }
                    else
                    {
                        current_action = decide_go_action(go_tick);
                    }
                }
                else if (state.phase == SBR2Phase::RESULT)
                {
                    bomb_placed_in_current_go = false;
                    ready_buttons = 0;
                    current_action = decide_result_action();
                }
                else
                {
                    ready_buttons = 0;
                    current_action = SBR2Action::WAIT;
                }

                if (current_action != last_printed_action)
                {
                    std::cout << "[tick " << tick << "] action = "
                              << action_to_string(current_action) << std::endl;
                    last_printed_action = current_action;
                }

                if (tick % 30 == 0)
                {
                    std::cout << "[phase] " << phase_to_string(state.phase)
                              << " self=(" << state.self_x << "," << state.self_y << ")"
                              << " enemy=(" << state.enemy_x << "," << state.enemy_y << ")"
                              << " ready_buttons=" << ready_buttons
                              << " go_open_delay=" << (state.go_open_delay_active ? 1 : 0)
                              << " result_seen=" << (result_seen_once ? 1 : 0)
                              << std::endl;
                }

                last_phase = state.phase;
            }

            if (state.phase == SBR2Phase::READY)
            {
                sender.send_buttons(ready_buttons);
            }
            else
            {
                sender.send_action(current_action);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));

            ++tick;
            if (tick >= kRoundFrames * kRoundCount)
            {
                tick = 0;
            }
        }
        
        sender.send_action(SBR2Action::WAIT); });

    std::cin.get();

    std::cout << "stopping..." << std::endl;
    running.store(false);

    if (worker.joinable())
    {
        worker.join();
    }

    pad.release_all();
    pad.disconnect();

    std::cout << "done" << std::endl;
    return 0;
}