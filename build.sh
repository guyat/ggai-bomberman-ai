#!/bin/bash

set -e

echo "=== Build sbr2_action_sender_test ==="
g++ -std=c++17 -I. \
test/sbr2_action_sender_test.cpp \
core/sbr2_action_sender.cpp \
core/sbr2_virtual_pad.cpp \
-lole32 -lsetupapi \
-o sbr2_action_sender_test.exe

echo "=== Build sbr2_ai_pad_test ==="
g++ -std=c++17 -I. \
test/sbr2_ai_pad_test.cpp \
core/sbr2_ai_brain.cpp \
core/sbr2_pathfinder.cpp \
core/sbr2_simulator.cpp \
core/sbr2_action_sender.cpp \
core/sbr2_virtual_pad.cpp \
core/sbr2_dummy_game_state_provider.cpp \
core/sbr2_vision_game_state_provider.cpp \
core/sbr2_screen_capture.cpp \
-lole32 -lsetupapi -lgdi32 \
-o sbr2_ai_pad_test.exe

echo "=== Build done ==="