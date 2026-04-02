#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdint>

#include "core/sbr2_virtual_pad.h"
#include "core/sbr2_action_sender.h"

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
        while (running.load()) {
            // 1) UP + LEFT
            std::cout << "send_buttons=5 (UP+LEFT)" << std::endl;
            for (int i = 0; i < 60 && running.load(); ++i) {
                sender.send_buttons((1u << 0) | (1u << 2)); // 5
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            // 少しニュートラル
            std::cout << "send_buttons=0 (NEUTRAL)" << std::endl;
            for (int i = 0; i < 20 && running.load(); ++i) {
                sender.send_buttons(0);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            // 2) UP + RIGHT
            std::cout << "send_buttons=9 (UP+RIGHT)" << std::endl;
            for (int i = 0; i < 60 && running.load(); ++i) {
                sender.send_buttons((1u << 0) | (1u << 3)); // 9
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            std::cout << "send_buttons=0 (NEUTRAL)" << std::endl;
            for (int i = 0; i < 20 && running.load(); ++i) {
                sender.send_buttons(0);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            // 3) DOWN + LEFT
            std::cout << "send_buttons=6 (DOWN+LEFT)" << std::endl;
            for (int i = 0; i < 60 && running.load(); ++i) {
                sender.send_buttons((1u << 1) | (1u << 2)); // 6
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            std::cout << "send_buttons=0 (NEUTRAL)" << std::endl;
            for (int i = 0; i < 20 && running.load(); ++i) {
                sender.send_buttons(0);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            // 4) DOWN + RIGHT
            std::cout << "send_buttons=10 (DOWN+RIGHT)" << std::endl;
            for (int i = 0; i < 60 && running.load(); ++i) {
                sender.send_buttons((1u << 1) | (1u << 3)); // 10
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            std::cout << "send_buttons=0 (NEUTRAL)" << std::endl;
            for (int i = 0; i < 20 && running.load(); ++i) {
                sender.send_buttons(0);
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
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