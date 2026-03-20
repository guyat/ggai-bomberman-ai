# ============================================
# GGAI / SBR2 AI 開発 現在状況まとめ（最重要）
# このブロックは README 先頭固定用
# ============================================

## まず最初に読むこと

この README は、長い会話ログに依存せず、次スレ移行時でも **100%に近い精度で続きから再開するための固定資産** として使う。

次スレで作業を再開する場合は、必ず以下を行うこと。

## リポジトリ

Repository:  
`https://github.com/guyat/ggai-bomberman-ai`

Working Directory:  
`/c/Users/PC_User/Documents/GGAI/ggai`

---

1. この README.md を最初から読む
2. 以下のファイルをリポジトリから読む（重要度順）ファイルは直前にGitへコミットしているためローカルとリポジトリのファイル内容は一致しているものとする。
【最重要（必ず読む）】
`core/sbr2_ai_brain.cpp`
`core/sbr2_ai_brain.h`
`test/sbr2_ai_brain_test.cpp`

【重要（必要に応じて参照）】
`core/sbr2_board.h`

【補助（挙動確認・理解用）】
`core/sbr2_pathfinder.cpp`
`core/sbr2_pathfinder.h`
`core/sbr2_simulator.cpp`
`core/sbr2_simulator.h`
`test/sbr2_pathfinder_test.cpp`
`test/sbr2_simulator_test.cpp`
3. その上で、README の「現在位置」と「次にやること」から再開する

---

## プロジェクトの目的（現行版）

Steam版（PC版）**Super Bomberman R2 (SBR2)** の **1vs1用AI** を開発するプロジェクト。

この AI は  
**SCP Virtual Bus Driver（scp_driver）による仮想コントローラー入力** でゲームを操作することを前提とする。

目的は単なる強AIではなく、

- フレーム単位で未来を読む
- 危険を回避する
- 爆弾を安全に置く
- 直線キル・予測攻撃・トラップを行う
- 将来的には人間上級者らしいテクニックまで扱う
- 最終的には「強い」だけでなく **人間っぽい** AI にする

ことである。

---

## 開発前提（超重要）

開発者（ユーザー）はプログラミングに不慣れであるため、今後の作業指示では以下を必須とする。

- どのファイルを直すか明示する
- 丸ごと置き換えか一部修正か明示する
- コピペしやすい形で出す
- できるだけ初心者でも迷わないように書く
- 「この文字列を検索してください」のように、検索しやすい案内を優先する


# ============================================

# GGAI / SBR2 AI 開発 現在状況まとめ（最重要・更新版）

# ============================================

## ■ このREADMEの役割

この README は、スレ移行しても **100%続きから再開できるための固定資産**。

---

# ■ 現在位置（2026年3月 最新）

## 🎯 開発フェーズ

### 完了済み

* 回避AI
* 爆弾設置AI
* 直線キル
* 1歩先予測
* Trap基礎
* guided_trap / guided_trap_v2
* close_range_pressure
* checkmate基礎
* Lv差 / Style差の土台

### 現在の段階（ここが重要）

👉 **AIの性格設計（Style差 + Level差）が成立した状態**

---

# ■ ステージ仕様（超重要）

* サイズ：**13 × 11**
* 左上：(0,0)
* 外周：壁
* ソフトブロック：なし

## ハードブロック

```cpp
(x % 2 == 1 && y % 2 == 1)
```

## ⚠ テスト時の最重要注意

* ハードブロック座標に立たせない
* `(5,5), (7,7)` はNG

---

# ■ AIの性格（最重要）

## Aggressive

* 高Lvで攻める
* 行動：

  * straight_kill
  * guided_bomb
  * close_range_pressure

## Careful

* 慎重型
* 特徴：

  * 一直線：**距離4以上で攻撃抑制**
  * 斜め：基本WAIT
  * 高Lvでも慎重維持

## Tricky

* 罠・誘導型
* 行動：

  * 一直線：guided_trap_v2
  * 斜め：guided_bomb
* 特徴：

  * 直線キルを部分的に抑制
  * 読み合い寄り

---

# ■ レベル差（AI Lv）

## 共通

* Lv5 → WAIT寄り
* Lv20 → 攻撃分岐解放

## Aggressive

* Lv5 → WAIT
* Lv20 → guided_bomb / straight_kill

## Careful

* Lv5 → WAIT
* Lv20 → 基本WAIT維持（一直線のみ攻撃）

## Tricky

* Lv5 → WAIT
* Lv20 → 誘導解禁

  * guided_trap_v2
  * guided_bomb

---

# ■ 今スレでの重要変更

## Tricky調整

```cpp
if (style() == SBR2AIStyle::Tricky && normalized_ai_level() >= 10)
```

### 効果

* 低Lvで trap封印
* 高Lvのみ罠AI化

---

# ■ 観測結果まとめ（超重要）

## CASE38（斜め中距離）

* Aggressive Lv20 → guided_bomb
* Careful → WAIT
* Tricky Lv20 → guided_bomb
* Lv5は全体WAIT

👉 **style差 + level差が成立**

---

## CASE39（一直線）

* Aggressive Lv20 → straight_kill
* Careful Lv20 → straight_kill
* Tricky Lv20 → guided_trap_v2
* Lv5は全体WAIT

👉 **Trickyだけ別行動**

---

# ■ 現在のAI構造理解

* style → 行動方針
* level → 行動解放

---

# ■ 現在の完成度

* style差：成立
* level差：成立
* Tricky個性：成立
* Careful慎重性：成立
* Aggressive攻撃性：成立

👉 **AIの性格設計は完成段階**

---

# ■ 次にやること（次スレ）

## 方針

👉 大きな構造変更はしない

## やること

* 実戦挙動確認
* 調整（微修正）
* scp_driver接続準備

---

# ■ 次スレ開始テンプレ（必須）

このプロジェクトの続きです。
README.mdとコードを読んで現状整理してください。

GitHub:
https://github.com/guyat/ggai-bomberman-ai

作業ディレクトリ:
`/c/Users/PC_User/Documents/GGAI/ggai`

読むファイル:

* core/sbr2_ai_brain.cpp
* core/sbr2_ai_brain.h
* core/sbr2_board.h
* test/sbr2_ai_brain_test.cpp

重要:

* いきなりコードを書かない
* まず現状整理
* 小さい変更のみ
* コピペ形式で指示
* エラーは最優先修正

