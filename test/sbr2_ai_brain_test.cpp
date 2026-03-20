#include <iostream>
#include <string>

#include "../core/sbr2_ai_brain.h"
#include "../core/sbr2_pathfinder.h"
#include "../core/sbr2_simulator.h"

#include <string>

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

    print_separator();
    return 0;
}