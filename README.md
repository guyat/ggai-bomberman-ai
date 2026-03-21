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

# GGAI / SBR2 AI 開発 現在状況まとめ（更新版・2026/03）

# ============================================

## ■ このREADMEの役割（再確認）

この README は
👉 **スレ移行しても100%続きから再開するための固定資産**

---

# ■ 現在位置（このスレ終了時点）

## 🎯 開発フェーズ

### 完了済み（確定）

* 回避AI（危険マス回避）
* 爆弾設置AI
* 直線キル
* guided_bomb
* guided_trap / guided_trap_v2
* close_range_pressure
* checkmate基礎
* Style差（Aggressive / Careful / Tricky）
* Level差（Lv5 / Lv20）
* TrickyのLv制御
* Carefulの慎重制御

---

## 🧠 今スレでやったこと（超重要）

### ① 不自然WAIT潰し

* Aggressive / Tricky が攻められるのに WAIT する問題を検証
* CASE40〜42で確認

👉 結論
**致命的な不自然WAITはなし（現状OK）**

---

### ② 再配置（move_toward_enemy）改善

#### 問題

* 上下往復（UP/DOWNループ）
* 横に寄らない
* 壁に擦る可能性

#### 対応

### ✔ 横優先ロジック修正

```cpp
abs(dy) > abs(dx) のときのみ縦上書き
```

👉 横に寄れるようになった

---

### ✔ fallback改善

* 「通れるだけ」→「敵に近づく方向」へ変更
* 同点時は直前方向優先

👉 無意味な往復削減

---

### ✔ 逆方向抑制

```cpp
reverse_of_last
```

👉 UP⇔DOWN往復を抑制

---

### ✔ 近距離揺れ制御

* 近距離（距離3以下）でswap抑制

---

### ✔ 行き過ぎ防止

* 同じ列・行での overshoot 抑制

---

## 🎯 結果（最重要）

### CASE48 / CASE49（最終確認）

👉 追従結果

* 横方向へ自然に寄る
* 壁に擦らない
* 無意味な往復なし
* 敵付近で自然停止

👉 結論

**再配置ロジックは実用レベルに到達**

---

# ■ 現在のAI構造

## 基本構造

* style → 行動方針
* level → 行動解放

---

## Aggressive

* 攻撃重視
* close_range_pressure
* straight_kill
* guided_bomb

---

## Careful

* 慎重
* 斜め基本WAIT
* 一直線のみ限定攻撃

---

## Tricky

* 誘導・罠
* guided_trap_v2
* 直線キル抑制

---

# ■ ステージ仕様（再確認）

* サイズ：13 × 11
* 左上：(0,0)
* ハードブロック：

```cpp
(x % 2 == 1 && y % 2 == 1)
```

## ⚠ テスト注意（最重要）

* ハードブロックに立たせない
* (5,5), (7,7) NG

---

# ■ 現在の完成度

| 項目      | 状態   |
| ------- | ---- |
| style差  | 完成   |
| level差  | 完成   |
| 攻撃AI    | 実用   |
| 再配置     | 実用   |
| 不自然WAIT | 問題なし |
| 安定性     | 高い   |

👉 **AIコアは完成段階に到達**

---

# ■ 次にやること（次スレ開始点）

## 🎯 フェーズ

👉 **実戦対応フェーズへ移行**

---

## ■ 次にやること（最優先）

### ① 実戦挙動確認

* 細かい違和感の洗い出し
* 微調整

---

### ② 防御拡張（重要）

👉 ここからが次の主軸

#### 追加予定

* 爆弾挟まれ時の脱出

  * パンチ脱出
  * キック脱出

#### 位置づけ

```
通常回避
↓
無理なら
↓
盤面操作で脱出（NEW）
```

👉 ギンギンパワーにおいて最重要防御

---

### ③ 上級テクニック拡張

今後追加予定：

```cpp
bool enable_bomb_tail = false;
bool enable_throw_chain = false;
bool enable_punch_chain = false;
bool enable_kick_chain = false;
bool enable_double_place_punch = false;
bool enable_throw_death_scythe = false;
bool enable_punch_death_scythe = false;
bool enable_kick_death_scythe = false;
bool enable_exploding_punch = false;
bool enable_right_timing_exploding_punch = false;
```

👉 **チェックボックス形式でON/OFF可能にする**

---

### ④ scp_driver接続準備

* 仮想コントローラー操作
* 実機テストへ移行

---

# ■ 開発方針（次スレ）

* 大規模変更禁止
* 小さい修正のみ
* 必ずテストで確認
* コピペ形式で指示

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

---

# ■ 現在の最重要ポイント（まとめ）

👉 AIコアは完成レベル
👉 次は「防御拡張」と「実機対応」

---

# ============================================
