#include <iostream>
#include "../core/sbr2_simulator.h"

int main()
{
    SBR2Simulator sim;

    // 通路マスに置く
    sim.add_bomb(2, 2, 0);
    sim.simulate();

    const int frame = 153;

    std::cout << "danger map at frame " << frame << "\n";

    for (int y = 0; y < SBR2Board::HEIGHT; ++y) {
        for (int x = 0; x < SBR2Board::WIDTH; ++x) {
            if (sim.is_danger(frame, x, y)) {
                std::cout << "*";
            } else if (sim.board().is_hard_block(x, y)) {
                std::cout << "H";
            } else {
                std::cout << ".";
            }
        }
        std::cout << "\n";
    }

    return 0;
}