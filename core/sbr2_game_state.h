// このファイルの役割

// 将来ここに、

// 自分の位置
// 相手の位置
// READY / GO / RESULT

// を入れてAIに渡すための共通の型にする。

#pragma once

enum class SBR2Phase
{
    UNKNOWN,
    READY,
    GO,
    RESULT
};

struct SBR2GameState
{
    bool self_found = false;
    int self_x = -1;
    int self_y = -1;

    bool enemy_found = false;
    int enemy_x = -1;
    int enemy_y = -1;

    SBR2Phase phase = SBR2Phase::UNKNOWN;
};