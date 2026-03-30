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
2. 以下のファイルをリポジトリから読む  
3. その上で README の「現在位置」と「次にやること」から再開する  

### 最重要（必ず読む）
`core/sbr2_ai_brain.cpp`  
`core/sbr2_ai_brain.h`  
`test/sbr2_ai_brain_test.cpp`  

### 重要（必要に応じて参照）
`core/sbr2_board.h`  
`core/sbr2_pathfinder.cpp`  
`core/sbr2_pathfinder.h`  
`core/sbr2_simulator.cpp`  
`core/sbr2_simulator.h`  

### 参考資料（今回以降かなり重要）
`original_amaAI/`  
`reference_runtime/ama_v1_8_2/`  
`reference_runtime/scp_driver/`  

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
- 最終的には「強い」だけでなく **人間っぽい / 学習用としても価値のある** AI にする

ことである。

---

## 開発前提（超重要）

開発者（ユーザー）はプログラミングに不慣れであるため、今後の作業指示では以下を必須とする。

- どのファイルを直すか明示する
- 丸ごと置き換えか一部修正か明示する
- コピペしやすい形で出す
- できるだけ初心者でも迷わないように書く
- 「この文字列を検索してください」のように、検索しやすい案内を優先する
- エラーが出たら最優先で修正する

---

# ============================================
# GGAI / SBR2 AI 開発 現在状況まとめ（更新版・2026/03/30）
# ============================================

## ■ このREADMEの役割（再確認）

この README は  
👉 **スレ移行しても100%続きから再開するための固定資産**

---

## ■ 現在位置（このスレ終了時点）

### 開発フェーズ
👉 **AIコア + 防御拡張 + level gate 実装完了 / 次は仮想コントローラー接続フェーズ**

---

## ■ ここまでで完了済み（重要）

### AIコア
- 回避AI（危険マス回避）
- 爆弾設置AI
- 直線キル
- guided_bomb
- guided_trap / guided_trap_v2
- close_range_pressure
- checkmate基礎
- Style差（Aggressive / Careful / Tricky）
- Level差（Lv5 / Lv20）
- TrickyのLv制御
- Carefulの慎重制御
- 再配置（move_toward_enemy）改善
- 不自然WAIT潰し

### 防御拡張
- 囲われ脱出パンチ 4方向
  - `PUNCH_RIGHT`
  - `PUNCH_LEFT`
  - `PUNCH_UP`
  - `PUNCH_DOWN`
- 囲われ脱出キック 4方向
  - `KICK_RIGHT`
  - `KICK_LEFT`
  - `KICK_UP`
  - `KICK_DOWN`
- 包囲キックストップ 4方向
  - `KICK_STOP_UP`
  - `KICK_STOP_LEFT`
  - `KICK_STOP_RIGHT`
  - `KICK_STOP_DOWN`
- delayed enclosure の最小入口
  - `KICK_STOP_DELAYED_RIGHT`
- 右方向の裏パンチ回避
  - 端の爆弾
  - 端1マス前の爆弾
  を `PUNCH_RIGHT` 候補として扱う最小版

### level gate
- 脱出技（パンチ + キック）: **Lv10以上**
- 包囲キックストップ: **Lv15以上**
- delayed enclosure: **Lv18以上**

### 参考資料整理
- `original_amaAI/` を repo 内に復元済み
- `reference_runtime/ama_v1_8_2/` を追加済み
- `reference_runtime/scp_driver/` を追加済み

---

## ■ 今スレでやったこと（超重要）

### ① 防御拡張フェーズを進めた
「爆弾に挟まれたときのパンチ・キック脱出」を中心に実装。

#### 囲われ脱出パンチ
- CASE 50: `PUNCH_RIGHT`
- CASE 51: `PUNCH_LEFT`
- CASE 52: `PUNCH_UP`
- CASE 53: `PUNCH_DOWN`

#### 囲われ脱出キック
- CASE 54: `KICK_RIGHT`
- CASE 55: `KICK_LEFT`
- CASE 56: `KICK_UP`
- CASE 57: `KICK_DOWN`

#### 包囲キックストップ
- CASE 58: `KICK_STOP_UP`
- CASE 59: `KICK_STOP_LEFT`
- CASE 60: `KICK_STOP_RIGHT`
- CASE 61: `KICK_STOP_DOWN`

#### delayed enclosure 最小入口
- CASE 62: 4個並び包囲観測ケース（誤発動しない WAIT 観測）
- CASE 63: `KICK_STOP_DELAYED_RIGHT`

#### 裏パンチ回避
- CASE 64: 端1マス前の爆弾 → `PUNCH_RIGHT`
- CASE 65: 端そのものの爆弾 → `PUNCH_RIGHT`

---

### ② level gate を実装した
#### gate 設定
- 脱出技（パンチ + キック）: **Lv10以上**
- 包囲キックストップ: **Lv15以上**
- delayed enclosure: **Lv18以上**

#### 観測ケース
- CASE 50-LV
  - Lv9 では脱出技を使わない
  - Lv20 では `PUNCH_RIGHT`
- CASE 63-LV
  - Lv17 では delayed enclosure を使わない
  - Lv20 では `KICK_STOP_DELAYED_RIGHT`

---

### ③ amaAI と scp_driver の参考資料を調査した
#### 調査結果
- `original_amaAI/puyop/encode.h`
- `original_amaAI/puyop/main.cpp`
- `original_amaAI/core/fieldbit.cpp`
- `original_amaAI/ai/search/beam/eval.cpp`
- `original_amaAI/ai/gaze.cpp`

を確認したが、**仮想コントローラー接続コードは見当たらなかった**。

#### 現時点の判断
- 公開されている `original_amaAI` からは  
  **scp_driver への接続コードを直接は参考にできない可能性が高い**
- `reference_runtime/scp_driver/` 内のファイルは
  - `.dll`
  - `.inf`
  - `.cat`
  - `.sys`
  であり、**接続コードそのものではなくランタイム / ドライバ本体** と考えるのが自然
- よって、次フェーズは  
  **GGAI 用の最小仮想コントローラー接続層を自作する方針**  
  で進める

---

## ■ 現在のAI構造（重要）

### 基本構造
- style → 行動方針
- level → 行動解放
- escape / enclosure / delayed enclosure → 防御技解放

### Aggressive
- 攻撃重視
- close_range_pressure
- straight_kill
- guided_bomb
- 脱出技 / 包囲外しも level に応じて使用

### Careful
- 慎重
- 斜め基本WAIT
- 一直線のみ限定攻撃

### Tricky
- 誘導・罠
- guided_trap_v2
- 直線キル抑制

---

## ■ ステージ仕様（再確認）

- サイズ：13 × 11
- 左上：(0,0)
- ハードブロック：
```cpp
(x % 2 == 1 && y % 2 == 1)
```

### テスト注意（最重要）
- ハードブロックに立たせない
- 盤面配置は `core/sbr2_board.h` の仕様に合わせること

---

## ■ 爆弾・テクニック仕様メモ（今後重要）

### パンチ
- 基本は **3マス先** へ飛ぶ
- ただし端付近の特殊仕様あり
- **端から3マス目の爆弾**は **2マスしか飛ばない**
- 端、端1マス前の爆弾は **裏パンチ回避** の候補

### 投げ
- 基本は **5マス先** へ飛ぶ
- ただし端付近の特殊仕様あり
- **端から5マス目から投げると、裏へ行かず外周の壁端にぴったり着弾**
- ハードブロック経由で相手に飛ばしてピヨらせる
  **同ライン通路投げピヨらせ** は将来実装予定

### 包囲外し
- 2マス間隔で並んだ爆弾列から
  **最遅の爆弾（または遅めの爆弾）を1マスキックストップで外し、残りを先に爆発させて安全化する**
  という理解で進める
- 現在は delayed enclosure の**最小入口のみ**実装済み
- 3個 / 5個 / 6個…への一般化は **未実装**

---

## ■ 上級テク土台（settings 側）
`sbr2_ai_brain.h` の `SBR2AIBrainSettings` には、将来実装用のフラグ土台がある。

```cpp
    bool enable_bomb_tail = false; // ボムテイル
    bool enable_punch_stun = false; // パンチピヨらせ
    bool enable_corridor_stun_throw = false; // 同ライン通路投げピヨらせ
    bool enable_throw_chain = false; // 投げ誘爆
    bool enable_punch_chain = false; // パンチ誘爆
    bool enable_kick_chain = false; // 蹴り誘爆
    bool enable_double_place_punch = false; // 2個置きパンチ
    bool enable_throw_death_scythe = false; // 投げデスサイズ
    bool enable_punch_death_scythe = false; // パンチデスサイズ
    bool enable_kick_death_scythe = false; // 蹴りデスサイズ
    bool enable_exploding_punch = false; // 起爆パンチ
    bool enable_right_timing_exploding_punch = false; // 目押し起爆パンチ
```

---

## ■ 現在の完成度

| 項目 | 状態 |
| --- | --- |
| AIコア | 実用レベル |
| 回避AI | 実用レベル |
| 攻撃AI | 実用レベル |
| 再配置 | 実用レベル |
| 囲われ脱出 | 実装済み |
| 包囲キックストップ | 実装済み |
| delayed enclosure | 最小入口のみ |
| level gate | 実装済み |
| 仮想コントローラー接続 | **未実装** |

👉 **AIコア + 防御拡張はかなり前進。次の最大の山は仮想コントローラー接続。**

---

## ■ 次にやること（次スレ開始点）

### 最優先
👉 **GGAI 用の最小仮想コントローラー接続設計 / 実装**

### 方針
amaAI の公開ソースから接続コードを拾う前提は一旦やめ、  
**GGAI 用に最小の接続層を自作する** 方針で進める。

### 最初の目標
1. 仮想コントローラー初期化の入口を作る  
2. 固定入力だけ送れるか確認する  
   - `WAIT`
   - `RIGHT`
   - `PLACE_BOMB`
3. `SBR2Action` を入力へ変換する層を作る  
4. そのあと AI Brain とつなぐ  

### 仮想コントローラー接続で参照するもの
- `original_amaAI/`
- `reference_runtime/ama_v1_8_2/`
- `reference_runtime/scp_driver/`

### ただし現時点の判断
- 公開されている `original_amaAI` からは、接続コードが見つからない可能性が高い
- なので **参考資料としては参照するが、接続層そのものは自作前提** で進める

---

## ■ 開発方針（次スレ）
- 大規模変更禁止
- 小さい修正のみ
- 必ずテストで確認
- コピペ形式で指示
- エラーは最優先修正
- 仮想コントローラー接続は、まず**最小の固定入力確認**から始める

---

## ■ 次スレ開始テンプレ（必須）

このプロジェクトの続きです。  
README.mdとコードを読んで現状整理してください。

GitHub:  
https://github.com/guyat/ggai-bomberman-ai

作業ディレクトリ:  
`/c/Users/PC_User/Documents/GGAI/ggai`

読むファイル:
- `README.md`
- `core/sbr2_ai_brain.cpp`
- `core/sbr2_ai_brain.h`
- `core/sbr2_board.h`
- `test/sbr2_ai_brain_test.cpp`

参考資料:
- `original_amaAI/`
- `reference_runtime/ama_v1_8_2/`
- `reference_runtime/scp_driver/`

重要:
- いきなりコードを書かない
- まず現状整理
- 次にやることは **GGAI 用の最小仮想コントローラー接続設計**
- 小さい変更のみ
- コピペ形式で指示
- エラーは最優先修正

---

## ■ 現在の最重要ポイント（まとめ）

👉 AIコアはかなり進んだ  
👉 防御拡張もかなり進んだ  
👉 amaAI 公開ソースから接続コードをそのまま拾うのは難しそう  
👉 次の最大の山は **仮想コントローラー接続と実動作確認**  
👉 まずは **GGAI 用の最小接続層** を作る

# ============================================
