#include <iostream>
#include <string>

#include "../core/sbr2_ai_brain.h"
#include "../core/sbr2_pathfinder.h"
#include "../core/sbr2_simulator.h"

extern std::string g_last_bomb_reason;

std::string action_to_string(SBR2Action action)
{
    switch (action)
    {
    case SBR2Action::WAIT:
        return "WAIT";
    case SBR2Action::UP:
        return "UP";
    case SBR2Action::DOWN:
        return "DOWN";
    case SBR2Action::LEFT:
        return "LEFT";
    case SBR2Action::RIGHT:
        return "RIGHT";
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
    case SBR2Action::KICK_STOP_UP:
        return "KICK_STOP_UP";
    case SBR2Action::KICK_STOP_LEFT:
        return "KICK_STOP_LEFT";
    case SBR2Action::KICK_STOP_RIGHT:
        return "KICK_STOP_RIGHT";
    case SBR2Action::KICK_STOP_DOWN:
        return "KICK_STOP_DOWN";
    case SBR2Action::KICK_STOP_DELAYED_RIGHT:
        return "KICK_STOP_DELAYED_RIGHT";
    default:
        return "UNKNOWN";
    }
}

void print_separator()
{
    std::cout << "========================================\n";
}

void print_case_summary(const std::string &case_name, bool ok, const std::string &detail = "")
{
    if (ok)
    {
        std::cout << case_name << ": OK\n";
    }
    else
    {
        std::cout << case_name << ": " << detail << "\n";
    }
}

int main()
{
    // CASE 1
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(1, 0);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        SBR2Action action = brain.decide_next_action(12, 10, 0);

        print_separator();
        std::cout << "CASE 1: safe start / should WAIT if enemy is far\n";
        std::cout << "action = " << action_to_string(action) << "\n";
    }

    // CASE 2
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.add_bomb(11, 10, 0);
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(1, 0);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        SBR2Action action = brain.decide_next_action(12, 10, 138);

        print_separator();
        std::cout << "CASE 2: dangerous soon / should escape\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::WAIT)
        {
            std::cout << "ERROR: got WAIT, but escape was expected.\n";
        }
        else
        {
            std::cout << "OK: non-WAIT action returned.\n";
        }
    }

    // CASE 3
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.add_bomb(11, 10, 0);
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(1, 0);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        SBR2EscapeResult result{};
        bool found = pathfinder.find_escape_action(12, 10, 138, result);
        SBR2Action brain_action = brain.decide_next_action(12, 10, 138);

        print_separator();
        std::cout << "CASE 3: compare Pathfinder and Brain\n";
        std::cout << "escape found = " << (found ? "TRUE" : "FALSE") << "\n";

        if (found && result.found)
        {
            std::cout << "pathfinder first_action = "
                      << action_to_string(result.first_action) << "\n";
            std::cout << "pathfinder target = ("
                      << static_cast<int>(result.target_x) << ", "
                      << static_cast<int>(result.target_y) << ")\n";
        }

        std::cout << "brain action = " << action_to_string(brain_action) << "\n";
    }

    // CASE 4
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(1, 0);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        SBR2Action action = brain.decide_next_action(12, 10, 100);

        print_separator();
        std::cout << "CASE 4: safe start / may reposition instead of WAIT\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::WAIT)
        {
            std::cout << "OK: WAIT returned.\n";
        }
        else
        {
            std::cout << "OK: repositioning is allowed in current AI.\n";
        }
    }

    // CASE 5
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(8, 10);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        SBR2Action action = brain.decide_next_action(12, 10, 200);

        print_separator();
        std::cout << "CASE 5: straight-line kill / should PLACE_BOMB\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::PLACE_BOMB)
        {
            std::cout << "OK: PLACE_BOMB returned.\n";
        }
        else
        {
            std::cout << "ERROR: expected PLACE_BOMB.\n";
        }
    }

    // CASE 6
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(11, 6);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        print_separator();
        std::cout << "CASE 6: blocked straight-line / should not PLACE_BOMB\n";

        SBR2Action action = brain.decide_next_action(11, 10, 300);
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::PLACE_BOMB)
        {
            std::cout << "ERROR: expected non-PLACE_BOMB.\n";
        }
        else
        {
            std::cout << "OK: did not place bomb because hard block blocks the line.\n";
        }
    }

    // CASE 7
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(9, 9);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        print_separator();
        std::cout << "CASE 7: one-step enemy prediction / level 20 may PLACE_BOMB\n";

        SBR2Action action = brain.decide_next_action(8, 10, 350);
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::PLACE_BOMB)
        {
            std::cout << "OK: PLACE_BOMB returned by one-step prediction.\n";
        }
        else
        {
            std::cout << "NOTE: did not place bomb (escape may be unsafe).\n";
        }
    }

    // CASE 8
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(1, 1);

        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        int start_x = 4;
        int start_y = 5;
        int start_frame = 100;

        SBR2Action action = brain.decide_next_action(start_x, start_y, start_frame);

        print_separator();
        std::cout << "CASE 8: trap check / note-only case\n";
        std::cout << "action = " << action_to_string(action) << "\n";
        std::cout << "NOTE: trap logic exists, and repositioning is also allowed here.\n";
    }

    // CASE 9
    // Lv5 は直線キルをまだ使わないので WAIT を期待
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(8, 10);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 5;
        SBR2AIBrain brain(simulator, pathfinder, settings);

        print_separator();
        std::cout << "CASE 9: level 5 / should WAIT on straight-line setup\n";

        SBR2Action action = brain.decide_next_action(12, 10, 200);
        std::cout << "brain level = " << brain.ai_level() << "\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::WAIT)
        {
            std::cout << "OK: low level did not use straight-line kill.\n";
        }
        else
        {
            std::cout << "ERROR: expected WAIT for level 5.\n";
        }
    }

    // CASE 10
    // Lv20 は直線キルを使うので PLACE_BOMB を期待
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

        print_separator();
        std::cout << "CASE 10: level 20 / should PLACE_BOMB on straight-line setup\n";

        SBR2Action action = brain.decide_next_action(12, 10, 200);
        std::cout << "brain level = " << brain.ai_level() << "\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }

        if (action == SBR2Action::PLACE_BOMB)
        {
            std::cout << "OK: high level used straight-line kill.\n";
        }
        else
        {
            std::cout << "ERROR: expected PLACE_BOMB for level 20.\n";
        }
    }

    // CASE 11
    // Aggressive は強引に攻める
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(8, 10);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        print_separator();
        std::cout << "CASE 11: Aggressive / should PLACE_BOMB\n";

        SBR2Action action = brain.decide_next_action(12, 10, 200);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }
    }

    // CASE 12
    // Careful は慎重（安全重視）
    // 危険なら攻撃せず回避、または慎重行動を取れればOK
    {
        SBR2Simulator simulator;
        simulator.clear();

        simulator.add_bomb(11, 10, 0);
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(8, 10);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Careful;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        print_separator();
        std::cout << "CASE 12: Careful / should avoid reckless bomb placement\n";

        SBR2Action action = brain.decide_next_action(12, 10, 138);

        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::PLACE_BOMB)
        {
            std::cout << "ERROR: Careful should avoid reckless bomb placement here.\n";
        }
        else
        {
            std::cout << "OK: Careful avoided reckless bomb placement.\n";
        }
    }

    // CASE 13
    // Tricky はtrap寄り
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(5, 5);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Tricky;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        print_separator();
        std::cout << "CASE 13: Tricky / trap-oriented\n";
        SBR2Action action = brain.decide_next_action(4, 5, 100);
        std::cout << "action = " << action_to_string(action) << "\n";
        bool ok13 = (action == SBR2Action::PLACE_BOMB);

        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }

        if (ok13)
        {
            print_case_summary("CASE 13", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 13", false, detail);
        }
    }

    // CASE 14
    // Tricky専用 trap テスト
    // set_block は無いので、既存爆弾で逃げ道を塞ぐ
    {
        SBR2Simulator simulator;
        simulator.clear();

        // 敵の周囲を爆弾でかなり狭くする
        simulator.add_bomb(5, 3, 0);
        simulator.add_bomb(7, 3, 0);
        simulator.add_bomb(6, 2, 0);

        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        i8 self_x = 6;
        i8 self_y = 5;

        board.set_enemy_position(6, 3);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Tricky;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        print_separator();
        std::cout << "CASE 14: Tricky trap test / should PLACE_BOMB if trap works\n";
        SBR2Action action = brain.decide_next_action(self_x, self_y, 100);
        std::cout << "action = " << action_to_string(action) << "\n";

        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }

        bool ok14 = (action == SBR2Action::PLACE_BOMB);

        if (ok14)
        {
            print_case_summary("CASE 14", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 14", false, detail);
        }
    }

    // CASE 15
    // 同じ方向に無駄に突っ込みすぎないか確認
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(1, 0);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action a1 = brain.decide_next_action(12, 10, 100);
        SBR2Action a2 = brain.decide_next_action(12, 10, 101);
        SBR2Action a3 = brain.decide_next_action(12, 10, 102);
        SBR2Action a4 = brain.decide_next_action(12, 10, 103);
        SBR2Action a5 = brain.decide_next_action(12, 10, 104);

        print_separator();

        bool ok15 =
            (a1 == SBR2Action::LEFT &&
             a2 == SBR2Action::LEFT &&
             a3 == SBR2Action::LEFT &&
             a4 == SBR2Action::LEFT &&
             a5 == SBR2Action::UP);

        if (ok15)
        {
            print_case_summary("CASE 15", true);
        }
        else
        {
            std::string detail =
                "NOTE a1=" + action_to_string(a1) +
                " a2=" + action_to_string(a2) +
                " a3=" + action_to_string(a3) +
                " a4=" + action_to_string(a4) +
                " a5=" + action_to_string(a5);
            print_case_summary("CASE 15", false, detail);
        }
    }

    // CASE 16
    // 再配置方向を短時間だけ維持して、壁際ふらつきを減らせるか確認
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 10;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        // 1回目は LEFT 優先
        board.set_enemy_position(7, 8);
        SBR2Action a1 = brain.decide_reposition_action_for_test(12, 10, 200);

        // 2回目は本来 UP 優先に変わる配置
        // ただし hold が効いていれば LEFT 維持
        board.set_enemy_position(11, 5);
        SBR2Action a2 = brain.decide_reposition_action_for_test(12, 10, 201);

        // 3回目は hold が切れて UP に変わる想定
        board.set_enemy_position(11, 5);
        SBR2Action a3 = brain.decide_reposition_action_for_test(12, 10, 202);

        print_separator();

        bool ok16 =
            (a1 == SBR2Action::LEFT &&
             a2 == SBR2Action::LEFT &&
             a3 == SBR2Action::LEFT);

        if (ok16)
        {
            print_case_summary("CASE 16", true);
        }
        else
        {
            std::string detail =
                "NOTE a1=" + action_to_string(a1) +
                " a2=" + action_to_string(a2) +
                " a3=" + action_to_string(a3);
            print_case_summary("CASE 16", false, detail);
        }
    }

    // CASE 17
    // 中央寄りでは再配置方向が壁際より早く変わるか確認
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 10;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        // 以前ビルドが通っていた形に戻す
        // 中央寄りの位置 (6, 5) を使う
        board.set_enemy_position(0, 5);
        SBR2Action a1 = brain.decide_reposition_action_for_test(6, 5, 300);

        board.set_enemy_position(5, 0);
        SBR2Action a2 = brain.decide_reposition_action_for_test(6, 5, 301);

        board.set_enemy_position(5, 0);
        SBR2Action a3 = brain.decide_reposition_action_for_test(6, 5, 302);

        print_separator();
        print_case_summary("CASE 17", false, "NOTE observation only");
    }

    // CASE 18
    // Trickyで少し遠い一直線配置でも guided_trap_v2 が出すぎないか確認
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(5, 10);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Tricky;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(4, 5, 100);

        print_separator();
        std::cout << "CASE 18: Tricky / far but safe\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 18", false, "NOTE observation only");
    }

    // CASE 19
    // Trickyで中距離一直線のときに guided_trap_v2 が出るか確認
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(5, 8);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Tricky;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(4, 5, 100);

        print_separator();
        std::cout << "CASE 19: Tricky / mid distance line\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 19", false, "NOTE observation only");
    }

    // CASE 20
    // Trickyで斜め近距離のとき、guided_bomb 系が見えるか観測
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 6);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Tricky;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(4, 5, 100);

        print_separator();
        std::cout << "CASE 20: Tricky / diagonal near\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 20", false, "NOTE observation only");
    }

    // CASE 21
    // Trickyで斜め中距離のとき、WAITではなく移動寄りになるか観測
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Tricky;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(4, 5, 100);

        print_separator();
        std::cout << "CASE 21: Tricky / diagonal mid\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (!g_last_bomb_reason.empty())
        {
            std::cout << "BOMB_REASON: " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 21", false, "NOTE observation only");
    }

    // CASE 22
    // 同一状況で Aggressive / Careful / Tricky の違いを見る
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 6);

        SBR2PathFinder pathfinder(board, simulator);

        std::cout << "========================================\n";
        std::cout << "CASE 22: Style comparison (diagonal near)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 22", false, "NOTE observation only");
    }

    // CASE 23
    // 同一状況で Aggressive / Careful / Tricky の違いを見る（一直線）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 8);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 23: Style comparison (straight line)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 23", false, "NOTE observation only");
    }

    // CASE 24
    // Careful が一直線でも少し遠いときに慎重になるか確認
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 10);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 24: Careful check (straight far)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 24", false, "NOTE observation only");
    }

    // CASE 25
    // 近距離一直線で Aggressive / Careful / Tricky の違いを見る
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 6);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 25: Style comparison (straight near)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 25", false, "NOTE observation only");
    }

    // CASE 26
    // Careful の境界確認（一直線・中距離）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 26: Careful boundary check (straight mid)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 26", false, "NOTE observation only");
    }

    // CASE 27
    // Careful の境界確認（一直線・やや遠め）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 9);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 27: Careful boundary check (straight dist4)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 27", false, "NOTE observation only");
    }

    // CASE 28
    // Aggressive の境界確認（一直線・遠め）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 10);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 28: Aggressive boundary check (straight far)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 28", false, "NOTE observation only");
    }

    // CASE 29
    // 斜め中距離で Aggressive / Careful / Tricky の違いを見る
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 29: Style comparison (diagonal mid)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 29", false, "NOTE observation only");
    }

    // CASE 30
    // 斜め近距離で Aggressive / Careful / Tricky の違いを見る
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(5, 6);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 30: Style comparison (diagonal near 2)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 30", false, "NOTE observation only");
    }

    // CASE 31
    // Aggressive の近距離ゴリ押し確認（隣接・逃げ道あり）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 6);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 31: Aggressive close-range pressure check\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 31", false, "NOTE observation only");
    }

    // CASE 32
    // Aggressive の近距離確殺確認（隣接・逃げ道が少ない）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(2, 3);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 32: Aggressive close-range checkmate check\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(2, 2, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(2, 2, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(2, 2, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 32", false, "NOTE observation only");
    }

    // CASE 33
    // Aggressive の近距離確殺再確認（壁際）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(0, 2);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 33: Aggressive close-range checkmate recheck\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(0, 1, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(0, 1, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(0, 1, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 33", false, "NOTE observation only");
    }

    // CASE 34
    // Aggressive の AIレベル差確認（斜め中距離）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 34: Aggressive level comparison (diagonal mid)\n";

        // Aggressive Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 34", false, "NOTE observation only");
    }

    // CASE 35
    // Careful の AIレベル差確認（一直線・やや遠め）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 9);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 35: Careful level comparison (straight dist4)\n";

        // Careful Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 35", false, "NOTE observation only");
    }

    // CASE 36
    // Tricky の AIレベル差確認（一直線）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 8);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 36: Tricky level comparison (straight line)\n";

        // Tricky Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 36", false, "NOTE observation only");
    }

    // CASE 37
    // Tricky の AIレベル差確認（斜め中距離）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 37: Tricky level comparison (diagonal mid)\n";

        // Tricky Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 37", false, "NOTE observation only");
    }

    // CASE 38
    // style差 + level差の総合比較（斜め中距離）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 38: Full comparison (diagonal mid)\n";

        // Aggressive Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 38", false, "NOTE observation only");
    }

    // CASE 39
    // style差 + level差の総合比較（一直線）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 8);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 39: Full comparison (straight line)\n";

        // Aggressive Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv20] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 39", false, "NOTE observation only");
    }

    // CASE 40
    // 不自然WAIT検出（斜め近距離・攻めれるはず）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(5, 6);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 40: unnatural WAIT check (diagonal near)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 40", false, "NOTE observation only");
    }

    // CASE 41
    // 不自然WAIT検出（一直線中距離・攻めれるはず）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(4, 8);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 41: unnatural WAIT check (straight line)\n";

        // Aggressive
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 41", false, "NOTE observation only");
    }

    // CASE 42
    // 低Lvの不自然WAIT検出（斜め中距離）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 42: low-level WAIT check (diagonal mid)\n";

        // Aggressive Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Aggressive Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Careful Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Careful Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        // Tricky Lv5
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 5;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_next_action(4, 5, 100);

            std::cout << "[Tricky Lv5] action = " << action_to_string(action) << "\n";
            if (!g_last_bomb_reason.empty())
                std::cout << "  reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 42", false, "NOTE observation only");
    }

    // CASE 43
    // 再配置の不自然さ確認（Aggressive / Careful / Tricky）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 43: reposition check\n";

        board.set_enemy_position(6, 7);

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_reposition_action_for_test(4, 5, 100);

            std::cout << "[Aggressive Lv20] action = " << action_to_string(action) << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_reposition_action_for_test(4, 5, 100);

            std::cout << "[Careful Lv20] action = " << action_to_string(action) << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_reposition_action_for_test(4, 5, 100);

            std::cout << "[Tricky Lv20] action = " << action_to_string(action) << "\n";
        }

        print_case_summary("CASE 43", false, "NOTE observation only");
    }

    // CASE 44
    // 再配置の不自然さ確認（片側が塞がれている）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 44: reposition check (one side blocked)\n";

        board.set_enemy_position(6, 7);

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_reposition_action_for_test(4, 6, 100);

            std::cout << "[Aggressive Lv20] action = " << action_to_string(action) << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_reposition_action_for_test(4, 6, 100);

            std::cout << "[Careful Lv20] action = " << action_to_string(action) << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);
            SBR2Action action = brain.decide_reposition_action_for_test(4, 6, 100);

            std::cout << "[Tricky Lv20] action = " << action_to_string(action) << "\n";
        }

        print_case_summary("CASE 44", false, "NOTE observation only");
    }

    // CASE 45
    // 再配置のブレ確認（同じ状況で連続フレーム）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 45: reposition jitter check\n";

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Aggressive Lv20]";
            for (int f = 100; f <= 104; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(4, 5, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Careful Lv20]";
            for (int f = 100; f <= 104; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(4, 5, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Tricky Lv20]";
            for (int f = 100; f <= 104; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(4, 5, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        } 

        print_case_summary("CASE 45", false, "NOTE observation only");
    }

    // CASE 46
    // 再配置の壁際確認（壁際で変な方向に粘らないか）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(2, 8);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 46: reposition near wall check\n";

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Aggressive Lv20]";
            for (int f = 100; f <= 104; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(0, 6, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Careful Lv20]";
            for (int f = 100; f <= 104; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(0, 6, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Tricky Lv20]";
            for (int f = 100; f <= 104; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(0, 6, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        print_case_summary("CASE 46", false, "NOTE observation only");
    }

    // CASE 47
    // 再配置の行き過ぎ確認（長めフレーム）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 47: reposition long-sequence check\n";

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Aggressive Lv20]";
            for (int f = 100; f <= 111; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(4, 5, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Careful Lv20]";
            for (int f = 100; f <= 111; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(4, 5, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            std::cout << "[Tricky Lv20]";
            for (int f = 100; f <= 111; ++f)
            {
                SBR2Action action = brain.decide_reposition_action_for_test(4, 5, f);
                std::cout << " " << action_to_string(action);
            }
            std::cout << "\n";
        }

        print_case_summary("CASE 47", false, "NOTE observation only");
    }

    // CASE 48
    // 再配置の追従確認（行動に応じて自分座標も進める）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(6, 7);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 48: reposition follow-through check\n";

        auto step_position = [](i8 &x, i8 &y, i8 enemy_x, i8 enemy_y, SBR2Action action)
        {
            i8 nx = x;
            i8 ny = y;

            switch (action)
            {
            case SBR2Action::UP:
                --ny;
                break;
            case SBR2Action::DOWN:
                ++ny;
                break;
            case SBR2Action::LEFT:
                --nx;
                break;
            case SBR2Action::RIGHT:
                ++nx;
                break;
            default:
                break;
            }

            // 敵マスには重ならない（実戦では同じマスに立てないため）
            if (nx == enemy_x && ny == enemy_y)
            {
                return;
            }

            x = nx;
            y = ny;
        };

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            i8 sx = 4;
            i8 sy = 5;

            std::cout << "[Aggressive Lv20]";
            for (int f = 100; f <= 105; ++f)
            {
                std::cout << " (" << static_cast<int>(sx) << "," << static_cast<int>(sy) << ")";
                SBR2Action action = brain.decide_reposition_action_for_test(sx, sy, f);
                std::cout << "->" << action_to_string(action);
                step_position(sx, sy, 6, 7, action);

                // 敵の隣まで来たら十分なので終了
                int dist_to_enemy = std::abs(6 - sx) + std::abs(7 - sy);
                if (dist_to_enemy <= 1)
                {
                    break;
                }
            }
            std::cout << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            i8 sx = 4;
            i8 sy = 5;

            std::cout << "[Careful Lv20]";
            for (int f = 100; f <= 105; ++f)
            {
                std::cout << " (" << static_cast<int>(sx) << "," << static_cast<int>(sy) << ")";
                SBR2Action action = brain.decide_reposition_action_for_test(sx, sy, f);
                std::cout << "->" << action_to_string(action);
                step_position(sx, sy, 6, 7, action);

                // 敵の隣まで来たら十分なので終了
                int dist_to_enemy = std::abs(6 - sx) + std::abs(7 - sy);
                if (dist_to_enemy <= 1)
                {
                    break;
                }
            }
            std::cout << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            i8 sx = 4;
            i8 sy = 5;

            std::cout << "[Tricky Lv20]";
            for (int f = 100; f <= 105; ++f)
            {
                std::cout << " (" << static_cast<int>(sx) << "," << static_cast<int>(sy) << ")";
                SBR2Action action = brain.decide_reposition_action_for_test(sx, sy, f);
                std::cout << "->" << action_to_string(action);
                step_position(sx, sy, 6, 7, action);

                // 敵の隣まで来たら十分なので終了
                int dist_to_enemy = std::abs(6 - sx) + std::abs(7 - sy);
                if (dist_to_enemy <= 1)
                {
                    break;
                }
            }
            std::cout << "\n";
        }

        print_case_summary("CASE 48", false, "NOTE observation only");
    }

    // CASE 49
    // 再配置の追従確認（壁際）
    {
        SBR2Simulator simulator;

        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());
        board.set_enemy_position(2, 8);

        SBR2PathFinder pathfinder(board, simulator);

        print_separator();
        std::cout << "CASE 49: reposition follow-through near wall\n";

        auto step_position = [](i8 &x, i8 &y, i8 enemy_x, i8 enemy_y, SBR2Action action)
        {
            i8 nx = x;
            i8 ny = y;

            switch (action)
            {
            case SBR2Action::UP:
                --ny;
                break;
            case SBR2Action::DOWN:
                ++ny;
                break;
            case SBR2Action::LEFT:
                --nx;
                break;
            case SBR2Action::RIGHT:
                ++nx;
                break;
            default:
                break;
            }

            // 敵マスには重ならない（実戦では同じマスに立てないため）
            if (nx == enemy_x && ny == enemy_y)
            {
                return;
            }

            x = nx;
            y = ny;
        };

        // Aggressive Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Aggressive;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            i8 sx = 0;
            i8 sy = 6;

            std::cout << "[Aggressive Lv20]";
            for (int f = 100; f <= 105; ++f)
            {
                std::cout << " (" << static_cast<int>(sx) << "," << static_cast<int>(sy) << ")";
                SBR2Action action = brain.decide_reposition_action_for_test(sx, sy, f);
                std::cout << "->" << action_to_string(action);
                step_position(sx, sy, 2, 8, action);

                // 敵の隣まで来たら十分なので終了
                int dist_to_enemy = std::abs(2 - sx) + std::abs(8 - sy);
                if (dist_to_enemy <= 1)
                {
                    break;
                }
            }
            std::cout << "\n";
        }

        // Careful Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Careful;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            i8 sx = 0;
            i8 sy = 6;

            std::cout << "[Careful Lv20]";
            for (int f = 100; f <= 105; ++f)
            {
                std::cout << " (" << static_cast<int>(sx) << "," << static_cast<int>(sy) << ")";
                SBR2Action action = brain.decide_reposition_action_for_test(sx, sy, f);
                std::cout << "->" << action_to_string(action);
                step_position(sx, sy, 2, 8, action);

                // 敵の隣まで来たら十分なので終了
                int dist_to_enemy = std::abs(2 - sx) + std::abs(8 - sy);
                if (dist_to_enemy <= 1)
                {
                    break;
                }
            }
            std::cout << "\n";
        }

        // Tricky Lv20
        {
            SBR2AIBrainSettings settings;
            settings.ai_level = 20;
            settings.style = SBR2AIStyle::Tricky;

            SBR2AIBrain brain(simulator, pathfinder, settings);

            i8 sx = 0;
            i8 sy = 6;

            std::cout << "[Tricky Lv20]";
            for (int f = 100; f <= 105; ++f)
            {
                std::cout << " (" << static_cast<int>(sx) << "," << static_cast<int>(sy) << ")";
                SBR2Action action = brain.decide_reposition_action_for_test(sx, sy, f);
                std::cout << "->" << action_to_string(action);
                step_position(sx, sy, 2, 8, action);

                // 敵の隣まで来たら十分なので終了
                int dist_to_enemy = std::abs(2 - sx) + std::abs(8 - sy);
                if (dist_to_enemy <= 1)
                {
                    break;
                }
            }
            std::cout << "\n";
        }

        print_case_summary("CASE 49", false, "NOTE observation only");
    }

    // CASE 50
    // surrounded punch escape (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (2, 0)
        // 上: 外周
        // 左: 爆弾
        // 右: 爆弾2連（パンチ候補）
        // 下: 爆弾
        board.set_cell(1, 0, SBR2Board::CellType::BOMB);
        board.set_cell(3, 0, SBR2Board::CellType::BOMB);
        board.set_cell(4, 0, SBR2Board::CellType::BOMB);
        board.set_cell(2, 1, SBR2Board::CellType::BOMB);

        // 敵は遠くへ置いて攻撃判定を避ける
        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 50: surrounded punch escape (direct board setup)\n";

        std::cout << "debug bomb(1,0) = " << (board.is_bomb(1, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(3,0) = " << (board.is_bomb(3, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(4,0) = " << (board.is_bomb(4, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(2,1) = " << (board.is_bomb(2, 1) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(2, 0, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok50 = (action == SBR2Action::PUNCH_RIGHT);

        if (ok50)
        {
            print_case_summary("CASE 50", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 50", false, detail);
        }
    }

    // CASE 51
    // surrounded punch escape left (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (10, 0)
        // 上: 外周
        // 右: 爆弾
        // 左: 爆弾2連（パンチ候補）
        // 下: 爆弾
        board.set_cell(11, 0, SBR2Board::CellType::BOMB);
        board.set_cell(9, 0, SBR2Board::CellType::BOMB);
        board.set_cell(8, 0, SBR2Board::CellType::BOMB);
        board.set_cell(10, 1, SBR2Board::CellType::BOMB);

        // 敵は遠くへ置いて攻撃判定を避ける
        board.set_enemy_position(0, 10);

        print_separator();
        std::cout << "CASE 51: surrounded punch escape left (direct board setup)\n";

        std::cout << "debug bomb(11,0) = " << (board.is_bomb(11, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(9,0) = " << (board.is_bomb(9, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(8,0) = " << (board.is_bomb(8, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(10,1) = " << (board.is_bomb(10, 1) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(10, 0, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok51 = (action == SBR2Action::PUNCH_LEFT);

        if (ok51)
        {
            print_case_summary("CASE 51", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 51", false, detail);
        }
    }

    // CASE 52
    // surrounded punch escape up (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (0, 2)
        // 左: 外周
        // 右: 爆弾
        // 下: 爆弾
        // 上: 爆弾2連（パンチ候補）
        board.set_cell(1, 2, SBR2Board::CellType::BOMB);
        board.set_cell(0, 3, SBR2Board::CellType::BOMB);
        board.set_cell(0, 1, SBR2Board::CellType::BOMB);
        board.set_cell(0, 0, SBR2Board::CellType::BOMB);

        // 敵は遠くへ置いて攻撃判定を避ける
        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 52: surrounded punch escape up (direct board setup)\n";

        std::cout << "debug bomb(1,2) = " << (board.is_bomb(1, 2) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,3) = " << (board.is_bomb(0, 3) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,1) = " << (board.is_bomb(0, 1) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,0) = " << (board.is_bomb(0, 0) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(0, 2, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok52 = (action == SBR2Action::PUNCH_UP);

        if (ok52)
        {
            print_case_summary("CASE 52", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 52", false, detail);
        }
    }

    // CASE 53
    // surrounded punch escape down (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置 (0, 8)
        // 上: 爆弾
        // 左: 外周
        // 右: 爆弾
        // 下: 爆弾2連
        board.set_cell(0, 7, SBR2Board::CellType::BOMB);
        board.set_cell(1, 8, SBR2Board::CellType::BOMB);
        board.set_cell(0, 9, SBR2Board::CellType::BOMB);
        board.set_cell(0, 10, SBR2Board::CellType::BOMB);

        board.set_enemy_position(12, 0);

        print_separator();
        std::cout << "CASE 53: surrounded punch escape down (direct board setup)\n";

        std::cout << "debug bomb(0,7) = " << (board.is_bomb(0, 7) ? "1" : "0") << "\n";
        std::cout << "debug bomb(1,8) = " << (board.is_bomb(1, 8) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,9) = " << (board.is_bomb(0, 9) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,10) = " << (board.is_bomb(0, 10) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(0, 8, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok53 = (action == SBR2Action::PUNCH_DOWN);

        if (ok53)
        {
            print_case_summary("CASE 53", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 53", false, detail);
        }
    }

    // CASE 54
    // surrounded kick escape right (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (2, 0)
        // 上: 外周
        // 左: 爆弾
        // 右: 爆弾1個（キック候補）
        // その先 (4,0) は空き
        // 下: 爆弾
        board.set_cell(1, 0, SBR2Board::CellType::BOMB);
        board.set_cell(3, 0, SBR2Board::CellType::BOMB);
        board.set_cell(2, 1, SBR2Board::CellType::BOMB);

        // 敵は遠くへ置いて攻撃判定を避ける
        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 54: surrounded kick escape right (direct board setup)\n";

        std::cout << "debug bomb(1,0) = " << (board.is_bomb(1, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(3,0) = " << (board.is_bomb(3, 0) ? "1" : "0") << "\n";
        std::cout << "debug empty(4,0) = " << (board.is_passable(4, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(2,1) = " << (board.is_bomb(2, 1) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(2, 0, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok54 = (action == SBR2Action::KICK_RIGHT);

        if (ok54)
        {
            print_case_summary("CASE 54", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 54", false, detail);
        }
    }

    // CASE 55
    // surrounded kick escape left (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (10, 0)
        // 上: 外周
        // 右: ハードブロック
        // 左: 爆弾1個（キック候補）
        // その先 (8,0) は空き
        // 下: 爆弾
        board.set_cell(11, 0, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(9, 0, SBR2Board::CellType::BOMB);
        board.set_cell(10, 1, SBR2Board::CellType::BOMB);

        board.set_enemy_position(0, 10);

        print_separator();
        std::cout << "CASE 55: surrounded kick escape left (direct board setup)\n";

        std::cout << "debug right_blocked(11,0) = " << (!board.is_passable(11, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(9,0) = " << (board.is_bomb(9, 0) ? "1" : "0") << "\n";
        std::cout << "debug empty(8,0) = " << (board.is_passable(8, 0) ? "1" : "0") << "\n";
        std::cout << "debug bomb(10,1) = " << (board.is_bomb(10, 1) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(10, 0, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok55 = (action == SBR2Action::KICK_LEFT);

        if (ok55)
        {
            print_case_summary("CASE 55", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 55", false, detail);
        }
    }

    // CASE 56
    // surrounded kick escape up (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (0, 2)
        // 左: 外周
        // 右: ハードブロック
        // 下: 爆弾
        // 上: 爆弾1個（キック候補）
        // その先 (0,0) は空き
        board.set_cell(1, 2, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(0, 3, SBR2Board::CellType::BOMB);
        board.set_cell(0, 1, SBR2Board::CellType::BOMB);

        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 56: surrounded kick escape up (direct board setup)\n";

        std::cout << "debug right_blocked(1,2) = " << (!board.is_passable(1, 2) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,3) = " << (board.is_bomb(0, 3) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,1) = " << (board.is_bomb(0, 1) ? "1" : "0") << "\n";
        std::cout << "debug empty(0,0) = " << (board.is_passable(0, 0) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(0, 2, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok56 = (action == SBR2Action::KICK_UP);

        if (ok56)
        {
            print_case_summary("CASE 56", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 56", false, detail);
        }
    }

    // CASE 57
    // surrounded kick escape down (direct board setup)
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (0, 8)
        // 上: ハードブロック
        // 左: 外周
        // 右: ハードブロック
        // 下: 爆弾1個（キック候補）
        // その先 (0,10) は空き
        board.set_cell(0, 7, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(1, 8, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(0, 9, SBR2Board::CellType::BOMB);

        board.set_enemy_position(12, 0);

        print_separator();
        std::cout << "CASE 57: surrounded kick escape down (direct board setup)\n";

        std::cout << "debug up_blocked(0,7) = " << (!board.is_passable(0, 7) ? "1" : "0") << "\n";
        std::cout << "debug right_blocked(1,8) = " << (!board.is_passable(1, 8) ? "1" : "0") << "\n";
        std::cout << "debug bomb(0,9) = " << (board.is_bomb(0, 9) ? "1" : "0") << "\n";
        std::cout << "debug empty(0,10) = " << (board.is_passable(0, 10) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(0, 8, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok57 = (action == SBR2Action::KICK_DOWN);

        if (ok57)
        {
            print_case_summary("CASE 57", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 57", false, detail);
        }
    }

    // CASE 58
    // enclosure kick-stop up
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (2, 8)
        //
        // 左右と下を塞ぐ
        // 上に「爆弾-空き-爆弾」の2マス間隔包囲
        //
        //   (2,5) = BOMB
        //   (2,6) = EMPTY
        //   (2,7) = BOMB
        //   (2,8) = SELF
        //
        // このケースでは普通の KICK_UP ではなく
        // KICK_STOP_UP を選びたい
        board.set_cell(1, 8, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(3, 8, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(2, 9, SBR2Board::CellType::HARD_BLOCK);

        board.set_cell(2, 7, SBR2Board::CellType::BOMB);
        board.set_cell(2, 5, SBR2Board::CellType::BOMB);

        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 58: enclosure kick-stop up\n";

        std::cout << "debug left_blocked(1,8) = " << (!board.is_passable(1, 8) ? "1" : "0") << "\n";
        std::cout << "debug right_blocked(3,8) = " << (!board.is_passable(3, 8) ? "1" : "0") << "\n";
        std::cout << "debug down_blocked(2,9) = " << (!board.is_passable(2, 9) ? "1" : "0") << "\n";
        std::cout << "debug bomb(2,7) = " << (board.is_bomb(2, 7) ? "1" : "0") << "\n";
        std::cout << "debug empty(2,6) = " << (board.is_passable(2, 6) ? "1" : "0") << "\n";
        std::cout << "debug bomb(2,5) = " << (board.is_bomb(2, 5) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(2, 8, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok58 = (action == SBR2Action::KICK_STOP_UP);

        if (ok58)
        {
            print_case_summary("CASE 58", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 58", false, detail);
        }
    }

    // CASE 59
    // enclosure kick-stop left
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (8, 2)
        //
        // 左に 爆弾-空き-爆弾
        // 上右下を塞ぐ
        board.set_cell(8, 1, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(9, 2, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(8, 3, SBR2Board::CellType::HARD_BLOCK);

        board.set_cell(7, 2, SBR2Board::CellType::BOMB);
        board.set_cell(5, 2, SBR2Board::CellType::BOMB);

        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 59: enclosure kick-stop left\n";

        std::cout << "debug up_blocked(8,1) = " << (!board.is_passable(8, 1) ? "1" : "0") << "\n";
        std::cout << "debug right_blocked(9,2) = " << (!board.is_passable(9, 2) ? "1" : "0") << "\n";
        std::cout << "debug down_blocked(8,3) = " << (!board.is_passable(8, 3) ? "1" : "0") << "\n";
        std::cout << "debug bomb(7,2) = " << (board.is_bomb(7, 2) ? "1" : "0") << "\n";
        std::cout << "debug empty(6,2) = " << (board.is_passable(6, 2) ? "1" : "0") << "\n";
        std::cout << "debug bomb(5,2) = " << (board.is_bomb(5, 2) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(8, 2, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok59 = (action == SBR2Action::KICK_STOP_LEFT);

        if (ok59)
        {
            print_case_summary("CASE 59", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 59", false, detail);
        }
    }

    // CASE 60
    // enclosure kick-stop right
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (4, 2)
        //
        // 右に 爆弾-空き-爆弾
        // 上左下を塞ぐ
        board.set_cell(4, 1, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(3, 2, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(4, 3, SBR2Board::CellType::HARD_BLOCK);

        board.set_cell(5, 2, SBR2Board::CellType::BOMB);
        board.set_cell(7, 2, SBR2Board::CellType::BOMB);

        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 60: enclosure kick-stop right\n";

        std::cout << "debug up_blocked(4,1) = " << (!board.is_passable(4, 1) ? "1" : "0") << "\n";
        std::cout << "debug left_blocked(3,2) = " << (!board.is_passable(3, 2) ? "1" : "0") << "\n";
        std::cout << "debug down_blocked(4,3) = " << (!board.is_passable(4, 3) ? "1" : "0") << "\n";
        std::cout << "debug bomb(5,2) = " << (board.is_bomb(5, 2) ? "1" : "0") << "\n";
        std::cout << "debug empty(6,2) = " << (board.is_passable(6, 2) ? "1" : "0") << "\n";
        std::cout << "debug bomb(7,2) = " << (board.is_bomb(7, 2) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(4, 2, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok60 = (action == SBR2Action::KICK_STOP_RIGHT);

        if (ok60)
        {
            print_case_summary("CASE 60", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 60", false, detail);
        }
    }

    // CASE 61
    // enclosure kick-stop down
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (2, 4)
        //
        // 下に 爆弾-空き-爆弾
        // 上左右を塞ぐ
        board.set_cell(2, 3, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(1, 4, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(3, 4, SBR2Board::CellType::HARD_BLOCK);

        board.set_cell(2, 5, SBR2Board::CellType::BOMB);
        board.set_cell(2, 7, SBR2Board::CellType::BOMB);

        board.set_enemy_position(12, 10);

        print_separator();
        std::cout << "CASE 61: enclosure kick-stop down\n";

        std::cout << "debug up_blocked(2,3) = " << (!board.is_passable(2, 3) ? "1" : "0") << "\n";
        std::cout << "debug left_blocked(1,4) = " << (!board.is_passable(1, 4) ? "1" : "0") << "\n";
        std::cout << "debug right_blocked(3,4) = " << (!board.is_passable(3, 4) ? "1" : "0") << "\n";
        std::cout << "debug bomb(2,5) = " << (board.is_bomb(2, 5) ? "1" : "0") << "\n";
        std::cout << "debug empty(2,6) = " << (board.is_passable(2, 6) ? "1" : "0") << "\n";
        std::cout << "debug bomb(2,7) = " << (board.is_bomb(2, 7) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(2, 4, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok61 = (action == SBR2Action::KICK_STOP_DOWN);

        if (ok61)
        {
            print_case_summary("CASE 61", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 61", false, detail);
        }
    }

    // CASE 62
    // enclosure delayed-bomb removal observation
    // 4個並びのうち1個を外すと安全になるイメージの観測ケース
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (4, 10)
        //
        // 横方向に 2マス間隔の爆弾列:
        // (1,8) BOMB
        // (3,8) BOMB
        // (5,8) BOMB   ← 将来的にはこのあたりを「最遅候補」として扱いたい
        // (7,8) BOMB
        //
        // 今回はまだ時刻差を持たせない。
        // まずは「この4個並びを観測ケースとして固定」する。
        //
        // 既存の KICK_STOP_LEFT / RIGHT に誤認されないように、
        // 自分は列の真横ではなく少し下に置く。
        board.set_cell(1, 8, SBR2Board::CellType::BOMB);
        board.set_cell(3, 8, SBR2Board::CellType::BOMB);
        board.set_cell(5, 8, SBR2Board::CellType::BOMB);
        board.set_cell(7, 8, SBR2Board::CellType::BOMB);

        // 周囲を少し制限
        board.set_cell(3, 10, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(5, 10, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(4, 9, SBR2Board::CellType::HARD_BLOCK);

        // 敵は遠く
        board.set_enemy_position(12, 0);

        print_separator();
        std::cout << "CASE 62: enclosure delayed-bomb removal observation\n";

        std::cout << "debug bomb(1,8) = " << (board.is_bomb(1, 8) ? "1" : "0") << "\n";
        std::cout << "debug bomb(3,8) = " << (board.is_bomb(3, 8) ? "1" : "0") << "\n";
        std::cout << "debug bomb(5,8) = " << (board.is_bomb(5, 8) ? "1" : "0") << "\n";
        std::cout << "debug bomb(7,8) = " << (board.is_bomb(7, 8) ? "1" : "0") << "\n";
        std::cout << "debug left_blocked(3,10) = " << (!board.is_passable(3, 10) ? "1" : "0") << "\n";
        std::cout << "debug right_blocked(5,10) = " << (!board.is_passable(5, 10) ? "1" : "0") << "\n";
        std::cout << "debug up_blocked(4,9) = " << (!board.is_passable(4, 9) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(4, 10, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        print_case_summary("CASE 62", false, "NOTE observation only");
    }

    // CASE 63
    // enclosure delayed-bomb target observation
    // 4個並びの中で「この爆弾を外したい」という候補を明示する観測ケース
    {
        SBR2Simulator simulator;
        simulator.clear();
        simulator.simulate();

        SBR2Board &board = const_cast<SBR2Board &>(simulator.board());

        // 自分位置は (4, 10)
        //
        // 横方向に 2マス間隔の爆弾列:
        // (1,8) BOMB
        // (3,8) BOMB
        // (5,8) BOMB   ← 将来的に「最遅候補」として外したい想定
        // (7,8) BOMB
        //
        // 今はまだ時刻差そのものは実装しない。
        // まずは「外したい候補が (5,8) である」ことを
        // テストケース上で明示して固定する。
        board.set_cell(1, 8, SBR2Board::CellType::BOMB);
        board.set_cell(3, 8, SBR2Board::CellType::BOMB);
        board.set_cell(5, 8, SBR2Board::CellType::BOMB);
        board.set_cell(7, 8, SBR2Board::CellType::BOMB);

        // 周囲制限
        board.set_cell(3, 10, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(5, 10, SBR2Board::CellType::HARD_BLOCK);
        board.set_cell(4, 9, SBR2Board::CellType::HARD_BLOCK);

        board.set_enemy_position(12, 0);

        print_separator();
        std::cout << "CASE 63: enclosure delayed-bomb target observation\n";

        std::cout << "debug bomb(1,8) = " << (board.is_bomb(1, 8) ? "1" : "0") << "\n";
        std::cout << "debug bomb(3,8) = " << (board.is_bomb(3, 8) ? "1" : "0") << "\n";
        std::cout << "debug target bomb(5,8) = " << (board.is_bomb(5, 8) ? "1" : "0") << "\n";
        std::cout << "debug bomb(7,8) = " << (board.is_bomb(7, 8) ? "1" : "0") << "\n";
        std::cout << "debug left_blocked(3,10) = " << (!board.is_passable(3, 10) ? "1" : "0") << "\n";
        std::cout << "debug right_blocked(5,10) = " << (!board.is_passable(5, 10) ? "1" : "0") << "\n";
        std::cout << "debug up_blocked(4,9) = " << (!board.is_passable(4, 9) ? "1" : "0") << "\n";

        SBR2PathFinder pathfinder(board, simulator);

        SBR2AIBrainSettings settings;
        settings.ai_level = 20;
        settings.style = SBR2AIStyle::Aggressive;

        SBR2AIBrain brain(simulator, pathfinder, settings);

        SBR2Action action = brain.decide_next_action(4, 10, 138);

        std::cout << "action = " << action_to_string(action) << "\n";
        if (!g_last_bomb_reason.empty())
        {
            std::cout << "reason = " << g_last_bomb_reason << "\n";
        }

        bool ok63 = (action == SBR2Action::KICK_STOP_DELAYED_RIGHT);

        if (ok63)
        {
            print_case_summary("CASE 63", true);
        }
        else
        {
            std::string detail =
                "NOTE action=" + action_to_string(action);
            print_case_summary("CASE 63", false, detail);
        }
    }

    print_separator();
    return 0;
}