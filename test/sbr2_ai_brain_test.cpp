#include <iostream>
#include <string>

#include "../core/sbr2_ai_brain.h"
#include "../core/sbr2_pathfinder.h"
#include "../core/sbr2_simulator.h"

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

        SBR2Action action = brain.decide_next_action(11, 10, 300);

        print_separator();
        std::cout << "CASE 6: blocked straight-line / should not PLACE_BOMB\n";
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

        SBR2Action action = brain.decide_next_action(8, 10, 350);

        print_separator();
        std::cout << "CASE 7: one-step enemy prediction / level 20 may PLACE_BOMB\n";
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

        SBR2Action action = brain.decide_next_action(12, 10, 200);

        print_separator();
        std::cout << "CASE 9: level 5 / should WAIT on straight-line setup\n";
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

        SBR2Action action = brain.decide_next_action(12, 10, 200);

        print_separator();
        std::cout << "CASE 10: level 20 / should PLACE_BOMB on straight-line setup\n";
        std::cout << "brain level = " << brain.ai_level() << "\n";
        std::cout << "action = " << action_to_string(action) << "\n";

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

        SBR2Action action = brain.decide_next_action(12, 10, 200);

        print_separator();
        std::cout << "CASE 11: Aggressive / should PLACE_BOMB\n";
        std::cout << "action = " << action_to_string(action) << "\n";
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

        SBR2Action action = brain.decide_next_action(12, 10, 138);

        print_separator();
        std::cout << "CASE 12: Careful / should avoid reckless bomb placement\n";
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

        SBR2Action action = brain.decide_next_action(4, 5, 100);

        print_separator();
        std::cout << "CASE 13: Tricky / trap-oriented\n";
        std::cout << "action = " << action_to_string(action) << "\n";
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

        SBR2Action action = brain.decide_next_action(self_x, self_y, 100);

        print_separator();
        std::cout << "CASE 14: Tricky trap test / should PLACE_BOMB if trap works\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::PLACE_BOMB)
        {
            std::cout << "OK: Tricky placed bomb for trap.\n";
        }
        else
        {
            std::cout << "NOTE: Tricky did not use trap in this setup.\n";
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
        std::cout << "CASE 15: avoid too much same-direction reposition\n";
        std::cout << "a1 = " << action_to_string(a1) << "\n";
        std::cout << "a2 = " << action_to_string(a2) << "\n";
        std::cout << "a3 = " << action_to_string(a3) << "\n";
        std::cout << "a4 = " << action_to_string(a4) << "\n";
        std::cout << "a5 = " << action_to_string(a5) << "\n";

        if (a1 == SBR2Action::LEFT &&
            a2 == SBR2Action::LEFT &&
            a3 == SBR2Action::LEFT &&
            a4 == SBR2Action::LEFT &&
            a5 == SBR2Action::UP)
        {
            std::cout << "OK: same-direction rush was softened on repeated calls.\n";
        }
        else
        {
            std::cout << "NOTE: result differed from expected pattern. Check streak control.\n";
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
        std::cout << "CASE 16: short hold for reposition direction\n";
        std::cout << "a1 = " << action_to_string(a1) << "\n";
        std::cout << "a2 = " << action_to_string(a2) << "\n";
        std::cout << "a3 = " << action_to_string(a3) << "\n";

        if (a1 == SBR2Action::LEFT &&
            a2 == SBR2Action::LEFT &&
            a3 == SBR2Action::LEFT)
        {
            std::cout << "OK: wall-side reposition hold stayed longer as expected.\n";
        }
        else
        {
            std::cout << "NOTE: result differed from expected wall-side hold pattern.\n";
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
        std::cout << "CASE 17: center-side reposition should change faster\n";
        std::cout << "a1 = " << action_to_string(a1) << "\n";
        std::cout << "a2 = " << action_to_string(a2) << "\n";
        std::cout << "a3 = " << action_to_string(a3) << "\n";

        std::cout << "NOTE: center-side behavior depends on actual movable directions on this tile.\n";
    }

    print_separator();
    return 0;
}