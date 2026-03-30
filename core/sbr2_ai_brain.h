#pragma once

#include <cstdint>

#include "sbr2_bomb.h"
#include "sbr2_pathfinder.h"
#include "sbr2_simulator.h"

using i8 = int8_t;
using i32 = int32_t;

// ★追加：AIスタイル
enum class SBR2AIStyle
{
    Aggressive,
    Careful,
    Tricky
};

struct SBR2AIBrainSettings
{
    int ai_level = 20;
    SBR2AIStyle style = SBR2AIStyle::Aggressive; // ★追加

    // 上級テク土台のみ(2026/03/27現在)
    bool enable_bomb_tail = false;//ボムテイル
    bool enable_punch_stun = false; // パンチピヨらせ
    bool enable_corridor_stun_throw = false;//同ライン通路投げピヨらせ
    bool enable_throw_chain = false;//投げ誘爆
    bool enable_punch_chain = false;//パンチ誘爆
    bool enable_kick_chain = false;//蹴り誘爆
    bool enable_double_place_punch = false;//2個置きパンチ
    bool enable_throw_death_scythe = false;//投げデスサイズ
    bool enable_punch_death_scythe = false;//パンチデスサイズ
    bool enable_kick_death_scythe = false;//蹴りデスサイズ
    bool enable_exploding_punch = false;//起爆パンチ
    bool enable_right_timing_exploding_punch = false;//目押し起爆パンチ
};

class SBR2AIBrain
{
public:
    SBR2AIBrain(
        const SBR2Simulator &simulator,
        const SBR2PathFinder &pathfinder);

    SBR2AIBrain(
        const SBR2Simulator &simulator,
        const SBR2PathFinder &pathfinder,
        const SBR2AIBrainSettings &settings);

    SBR2Action decide_next_action(i8 x, i8 y, i32 frame) const;
    SBR2Action decide_reposition_action_for_test(i8 x, i8 y, i32 frame) const;

    int ai_level() const;

private:
    const SBR2Simulator &simulator_;
    const SBR2PathFinder &pathfinder_;
    SBR2AIBrainSettings settings_;

    mutable int last_bomb_frame_ = -1000;
    mutable SBR2Action last_reposition_action_ = SBR2Action::WAIT;
    mutable int same_direction_reposition_count_ = 0;
    mutable SBR2Action held_reposition_action_ = SBR2Action::WAIT;
    mutable int held_reposition_until_frame_ = -1;

    bool is_move_action(SBR2Action action) const;
    int same_direction_reposition_limit(i8 x, i8 y) const;
    SBR2Action remember_reposition_action(SBR2Action action) const;
    SBR2Action reset_reposition_state_and_return(SBR2Action action) const;

    int reposition_hold_extra_frames(i8 x, i8 y) const;
    SBR2Action apply_reposition_hold(
        SBR2Action action, i8 x, i8 y, i32 frame) const;

    int normalized_ai_level() const;
    int bomb_cooldown_frames() const;

    bool level_allows_straight_kill() const;
    bool level_allows_one_step_prediction() const;
    bool level_allows_trap() const;

    bool level_allows_surrounded_escape() const;
    bool level_allows_enclosure_kick_stop() const;
    bool level_allows_delayed_enclosure_escape() const;

    int trap_survivable_cell_threshold() const;

    // ★追加：スタイル取得
    SBR2AIStyle style() const { return settings_.style; }

    bool will_be_dangerous_soon(i8 x, i8 y, i32 frame) const;
    bool can_place_bomb_and_escape(i8 x, i8 y, i32 frame) const;
    bool can_place_bomb_and_escape_strict(i8 x, i8 y, i32 frame) const;

    bool can_hit_enemy_in_straight_line(i8 x, i8 y, i8 ex, i8 ey) const;

    bool can_hit_enemy_now_or_after_one_step(
        i8 x,
        i8 y,
        bool allow_one_step_prediction) const;

    bool is_trap_possible(i8 self_x, i8 self_y, i32 frame) const;
    int count_enemy_survivable_cells_after_bomb(i8 self_x, i8 self_y, i32 frame) const;
    SBR2Action move_toward_enemy(i8 self_x, i8 self_y, i32 frame) const;
    bool can_step_to(i8 x, i8 y) const;
    SBR2Action fallback_safe_step(i8 self_x, i8 self_y) const;

    bool can_use_surrounded_punch_escape_right(i8 x, i8 y, i32 frame) const;
    bool can_use_surrounded_punch_escape_left(i8 x, i8 y, i32 frame) const;
    bool can_use_surrounded_punch_escape_up(i8 x, i8 y, i32 frame) const;
    bool can_use_surrounded_punch_escape_down(i8 x, i8 y, i32 frame) const;

    bool can_use_surrounded_kick_escape_right(i8 x, i8 y, i32 frame) const;
    bool can_use_surrounded_kick_escape_left(i8 x, i8 y, i32 frame) const;
    bool can_use_surrounded_kick_escape_up(i8 x, i8 y, i32 frame) const;
    bool can_use_surrounded_kick_escape_down(i8 x, i8 y, i32 frame) const;

    bool can_use_enclosure_kick_stop_up(i8 x, i8 y, i32 frame) const;
    bool can_use_enclosure_kick_stop_left(i8 x, i8 y, i32 frame) const;
    bool can_use_enclosure_kick_stop_right(i8 x, i8 y, i32 frame) const;
    bool can_use_enclosure_kick_stop_down(i8 x, i8 y, i32 frame) const;

    bool can_use_delayed_enclosure_kick_stop_right(i8 x, i8 y, i32 frame) const;
};