# ggai-bomberman-ai

# GGAI Bomberman AI

Steam版(PC版)Super Bomberman R2 の 1vs1 用 AI を開発するプロジェクト。

このAIは SCP Virtual Bus Driverによる**仮想コントローラー操作**(scp_driver)でゲームをプレイすることを目標としています。

相変わらずプログラミングはからっきしなので丁寧に、コピペで丸写し出来るような感じ多めでお願い致します。

---

AI project for **Steam version Super Bomberman R2 (1v1)**.

The goal of this project is to build a strong CPU opponent using  
**virtual controller input** and frame-level simulation.

This AI is currently focused on **explosion prediction and escape logic**.

---

# Current AI Status

Implemented systems

- Bomb simulator
- DangerMap (future explosion prediction)
- Time-aware BFS pathfinding
- Safe cell search
- Route reconstruction
- First action extraction
- AI Brain decision layer
- Future danger escape AI

The AI can already detect future explosions and escape safely.

---

# Repository Structure


ggai/
├ core/
│ ├ sbr2_ai_brain.cpp
│ ├ sbr2_ai_brain.h
│ ├ sbr2_board.h
│ ├ sbr2_bomb.h
│ ├ sbr2_pathfinder.cpp
│ ├ sbr2_pathfinder.h
│ ├ sbr2_simulator.cpp
│ └ sbr2_simulator.h
│
├ test/
│ ├ sbr2_simulator_test.cpp
│ ├ sbr2_pathfinder_test.cpp
│ └ sbr2_ai_brain_test.cpp
│
├ README.md
└ .gitignore


---

# Development Environment

OS


Windows


Shell


Git Bash (MINGW64)


Compiler


g++ (C++17)


Working directory


/c/Users/PC_User/Documents/GGAI/ggai


---

# Game Specifications

Game


Steam Super Bomberman R2


Mode


1 vs 1


Stage size


11 × 13 tiles


Blocks


Outer walls
Fixed hard blocks
No soft blocks


Bomb capacity


8 bombs


Explosion power


8 tiles


---

# Round Start Rule

SBR2 round start behavior


READY appears
↓
Players can move
↓
GO! appears
↓
Bomb placement becomes available


The AI must respect this rule.

---

# Frame Timing Reference

All timings were measured by analyzing  
**60 FPS video frame-by-frame**.

---

# Bomb Timing

Bomb placement


frame 0


Explosion


frame 153


Chain explosion delay


10 frames


Explosion spread


Instant propagation
Range = 8 tiles


---

# Player Movement Timing

Movement timing was measured from tile center.

### First movement from tile center


0F movement starts
1F
2F
3F
4F
5F still considered inside the original tile
6F reaches the next tile


Therefore:


center → adjacent tile = 6 frames


---

### Continuous movement

Once a player has reached the next tile:


0F tile reached
1F
2F
3F
4F
5F
6F
7F
8F
9F
10F still inside current tile
11F reaches the next tile


Therefore:


normal grid movement = 11 frames per tile


---

# Current AI Simplification

For the current implementation the pathfinder uses


MOVE_FRAMES = 11


for all movement calculations.

Future improvements may include:


center movement = 6F
continuous movement = 11F


to achieve more precise simulation.

---

# DangerMap

Explosion prediction horizon


512 frames


DangerMap predicts all explosion tiles over time and is used by the pathfinder.

---

# AI Architecture

Current decision pipeline


DangerMap
↓
Time-aware BFS
↓
Safe cell detection
↓
Escape route reconstruction
↓
AI Brain decision


The current AI prioritizes **survival and explosion avoidance**.

---

# Test Configuration

For reproducibility during development


AI spawn = bottom right
Enemy spawn = top left


Actual game spawn positions are random.

---

# Planned Future Features


Bomb placement AI
Enemy tracking
Trap logic
CPU difficulty levels (Lv1–Lv20)


Example CPU levels


Lv1 random movement
Lv5 basic bomb avoidance
Lv10 future danger escape
Lv15 bomb placement
Lv20 strategic trapping


---

# Project Goal

The long-term goal is to create a **high-level Bomberman CPU AI** capable of


prediction
escape
bomb placement
trapping opponents


using frame-level simulation.