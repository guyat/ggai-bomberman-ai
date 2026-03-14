#include <iostream>
#include <string>

#include "core/sbr2_simulator.h"
#include "core/sbr2_pathfinder.h"

// ======================================================
// テスト用開始位置
// 再現性のため固定
// 本番AIでは開始位置を固定前提にしないこと
// ======================================================
const i8 AI_START_X = 12;
const i8 AI_START_Y = 10;

const i8 ENEMY_START_X = 0;
const i8 ENEMY_START_Y = 0;

// ======================================================
// 1ケース分を表示するための補助関数
// ======================================================
void print_result(const std::string& test_name, bool result)
{
    std::cout << "========================================\n";
    std::cout << test_name << "\n";
    std::cout << "result = " << (result ? "TRUE" : "FALSE") << "\n";
    std::cout << "========================================\n\n";
}

int main()
{
    // --------------------------------------------------
    // CASE 1:
    // 爆弾なし。右下開始で安全地帯に行けるか
    // これは基本 TRUE になってほしい
    // --------------------------------------------------
    {
        SBR2Simulator sim;
        sim.clear();

        sim.simulate();

        SBR2PathFinder pathfinder(sim.board(), sim);

        bool reachable = pathfinder.can_reach_safe_cell(AI_START_X, AI_START_Y, 0);

        print_result("CASE 1: no bomb / should be reachable", reachable);
    }

    // --------------------------------------------------
    // CASE 2:
    // AIの近くに爆弾があるが、まだ逃げられる想定
    // 基本 TRUE を期待
    //
    // 右下 (12,10) 開始
    // 近くのマスに爆弾を置いて danger を作る
    // --------------------------------------------------
    {
        SBR2Simulator sim;
        sim.clear();

        sim.add_bomb(10, 10, 0);

        sim.simulate();

        SBR2PathFinder pathfinder(sim.board(), sim);

        bool reachable = pathfinder.can_reach_safe_cell(AI_START_X, AI_START_Y, 0);

        print_result("CASE 2: one nearby bomb / escape should be possible", reachable);

        i8 safe_x = -1;
        i8 safe_y = -1;
        bool found_safe_cell = pathfinder.find_safe_cell(AI_START_X, AI_START_Y, 0, safe_x, safe_y);

        std::cout << "CASE 2 safe cell found = "
                  << (found_safe_cell ? "TRUE" : "FALSE") << "\n";

        if (found_safe_cell) {
            std::cout << "CASE 2 safe cell = ("
                      << static_cast<int>(safe_x) << ", "
                      << static_cast<int>(safe_y) << ")\n";
        }

        std::cout << "\n";
    }

    // --------------------------------------------------
    // CASE 3:
    // 観測用ケース
    // 右下の2方向出口付近に爆弾を置く
    //
    // これが TRUE/FALSE どちらになるかを見る
    // もし TRUE なら「今のBFS仕様ではまだ逃げあり判定」
    // もし FALSE なら「出口封鎖が効いている」
    //
    // まずは仕様確認用のケースとして置く
    // --------------------------------------------------
    {
        SBR2Simulator sim;
        sim.clear();

        // AI開始位置 (12,10) の左と上に相当する出口側を圧迫
        // 左出口側
        sim.add_bomb(11, 10, 0);
        // 上出口側
        sim.add_bomb(12, 9, 0);

        sim.simulate();

        SBR2PathFinder pathfinder(sim.board(), sim);

        bool reachable = pathfinder.can_reach_safe_cell(AI_START_X, AI_START_Y, 0);

        print_result("CASE 3: corner exits pressured by bombs / observe result", reachable);
    }

        // --------------------------------------------------
    // CASE 4:
    // 明確に逃げられないケースを作りたい
    //
    // AI開始位置は (12,10)
    // 同じ行の左側に爆弾を置くと、爆風が右下開始位置まで届く
    // さらに開始フレームを爆発直前にして、
    // 11F移動では間に合わない状況を作る
    //
    // placed_frame = 0 の爆弾は 153F で爆発
    // start_frame = 152 から開始すると、
    // 次の1F後に爆発してしまうので逃げにくい
    //
    // 期待値: FALSE
    // --------------------------------------------------
    {
        SBR2Simulator sim;
        sim.clear();

        // (4,10) に置くと、右方向へ爆風が (12,10) まで届く
        sim.add_bomb(4, 10, 0);

        sim.simulate();

        SBR2PathFinder pathfinder(sim.board(), sim);

        // 爆発直前から開始
        bool reachable = pathfinder.can_reach_safe_cell(AI_START_X, AI_START_Y, 152);

        print_result("CASE 4: start just before explosion / should be unreachable", reachable);
    }

        // --------------------------------------------------
    // CASE 5:
    // 開始位置そのものをこのままだと危険にしつつ、
    // まだ別の安全マスへ逃げられるケースを観測する
    //
    // 目的:
    // find_safe_cell(...) が開始位置以外の座標を返すか確認する
    // --------------------------------------------------
    {
        SBR2Simulator sim;
        sim.clear();

        // (4,10) の爆風は右方向に伸びて (12,10) まで届く
        // ただし開始フレームを 141 にして、爆発(153F)まで少し余裕を持たせる
        // これなら「その場に居続けるのは危ないが、逃げ道はある」状況を期待する
        sim.add_bomb(4, 10, 0);

        sim.simulate();

        SBR2PathFinder pathfinder(sim.board(), sim);

        bool reachable = pathfinder.can_reach_safe_cell(AI_START_X, AI_START_Y, 141);

        print_result("CASE 5: start will become dangerous / should still escape", reachable);

        i8 safe_x = -1;
        i8 safe_y = -1;
        bool found_safe_cell = pathfinder.find_safe_cell(AI_START_X, AI_START_Y, 141, safe_x, safe_y);

        std::cout << "CASE 5 safe cell found = "
                  << (found_safe_cell ? "TRUE" : "FALSE") << "\n";

        if (found_safe_cell) {
            std::cout << "CASE 5 safe cell = ("
                      << static_cast<int>(safe_x) << ", "
                      << static_cast<int>(safe_y) << ")\n";
        }

        std::cout << "\n";
    }

        // --------------------------------------------------
    // CASE 6:
    // find_escape_action のテスト
    // 安全マスへの最初の行動を確認
    // --------------------------------------------------
    {
        SBR2Simulator sim;
        sim.clear();

        sim.add_bomb(4, 10, 0);

        sim.simulate();

        SBR2PathFinder pathfinder(sim.board(), sim);

        SBR2EscapeResult result{};

        bool found = pathfinder.find_escape_action(
            AI_START_X,
            AI_START_Y,
            141,
            result
        );

        std::cout << "========================================\n";
        std::cout << "CASE 6: escape action test\n";
        std::cout << "escape found = " << (found ? "TRUE" : "FALSE") << "\n";

        if (found) {
            std::cout << "escape target = ("
                      << static_cast<int>(result.target_x) << ", "
                      << static_cast<int>(result.target_y) << ")\n";

            std::cout << "first action = "
                      << static_cast<int>(result.first_action) << "\n";
        }

        std::cout << "========================================\n\n";
    }

    return 0;
}