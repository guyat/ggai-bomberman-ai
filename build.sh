#!/usr/bin/env bash
set -e

echo "== Build =="
g++ -std=c++17 -Icore \
  core/sbr2_simulator.cpp \
  core/sbr2_pathfinder.cpp \
  core/sbr2_ai_brain.cpp \
  test/sbr2_ai_brain_test.cpp \
  -o test_ai_brain

echo "== Run Tests =="
./test_ai_brain

echo "== Done =="

