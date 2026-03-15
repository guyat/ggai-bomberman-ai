#include <iostream>
#include <string>

#include "../core/sbr2_ai_brain.h"
#include "../core/sbr2_pathfinder.h"
#include "../core/sbr2_simulator.h"

std::string action_to_string(SBR2Action action)
{
    switch (action) {
    case SBR2Action::WAIT:  return "WAIT";
    case SBR2Action::UP:    return "UP";
    case SBR2Action::DOWN:  return "DOWN";
    case SBR2Action::LEFT:  return "LEFT";
    case SBR2Action::RIGHT: return "RIGHT";
    case SBR2Action::PLACE_BOMB: return "PLACE_BOMB";
    default:                return "UNKNOWN";
    }
}

void print_separator()
{
    std::cout << "========================================\n";
}

int main()
{
    // ----------------------------------------
    // CASE 1:
    // 危険ではない場所なら WAIT になるか
    // ----------------------------------------
    {
        SBR2Simulator simulator;
        simulator.clear();

        // 爆弾なし
        simulator.simulate();

        const SBR2Board& board = simulator.board();
        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        int start_x = 12;
        int start_y = 10;
        int start_frame = 0;

        SBR2Action action = brain.decide_next_action(start_x, start_y, start_frame);

        print_separator();
        std::cout << "CASE 1: safe start / should PLACE_BOMB if escape exists\n";
        std::cout << "action = " << action_to_string(action) << "\n";
    }

    // ----------------------------------------
    // CASE 2:
    // 未来で危険になる位置から、逃げ行動を返すか
    // 右下開始(12,10)、その左の通路(11,10)に爆弾
    // 爆弾は 153F で爆発
    // 開始を 145F にすると、8F後に爆発なので
    // WAIT だと危ない。逃げを返してほしい。
    // ----------------------------------------
    {
        SBR2Simulator simulator;
        simulator.clear();

        simulator.add_bomb(12, 9, 0);
        simulator.simulate();

        const SBR2Board& board = simulator.board();
        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        int start_x = 12;
        int start_y = 10;
        int start_frame = 138;

        SBR2Action action = brain.decide_next_action(start_x, start_y, start_frame);

        print_separator();
        std::cout << "CASE 2: dangerous soon / should escape\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::WAIT) {
            std::cout << "ERROR: got WAIT, but escape was expected.\n";
        } else {
            std::cout << "OK: non-WAIT action returned.\n";
        }
    }

    // ----------------------------------------
    // CASE 3:
    // Pathfinder の escape_action と AI Brain の結果を見比べる
    // ----------------------------------------
    {
        SBR2Simulator simulator;
        simulator.clear();

        simulator.add_bomb(12, 9, 0);
        simulator.simulate();

        const SBR2Board& board = simulator.board();
        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        int start_x = 12;
        int start_y = 10;
        int start_frame = 138;

        SBR2EscapeResult result{};
        bool found = pathfinder.find_escape_action(start_x, start_y, start_frame, result);
        SBR2Action brain_action = brain.decide_next_action(start_x, start_y, start_frame);

        print_separator();
        std::cout << "CASE 3: compare Pathfinder and Brain\n";
        std::cout << "escape found = " << (found ? "TRUE" : "FALSE") << "\n";

        if (found && result.found) {
            std::cout << "pathfinder first_action = "
                      << action_to_string(result.first_action) << "\n";
            std::cout << "pathfinder target = ("
                      << static_cast<int>(result.target_x) << ", "
                      << static_cast<int>(result.target_y) << ")\n";
        }

        std::cout << "brain action = " << action_to_string(brain_action) << "\n";
    }

        // ----------------------------------------
    // CASE 4:
    // 安全スタートなら PLACE_BOMB を返せるか
    // （置いた後に逃げ切れる場合のみ）
    // ----------------------------------------
    {
        SBR2Simulator simulator;
        simulator.clear();

        // 爆弾なし
        simulator.simulate();

        const SBR2Board& board = simulator.board();
        SBR2PathFinder pathfinder(board, simulator);
        SBR2AIBrain brain(simulator, pathfinder);

        int start_x = 12;
        int start_y = 10;
        int start_frame = 0;

        SBR2Action action = brain.decide_next_action(start_x, start_y, start_frame);

        print_separator();
        std::cout << "CASE 4: safe start / should PLACE_BOMB if escape exists\n";
        std::cout << "action = " << action_to_string(action) << "\n";

        if (action == SBR2Action::PLACE_BOMB) {
            std::cout << "OK: PLACE_BOMB returned.\n";
        } else {
            std::cout << "INFO: PLACE_BOMB was not returned.\n";
        }
    }

    print_separator();
    return 0;
}