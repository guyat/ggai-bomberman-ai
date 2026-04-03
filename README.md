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
`core/sbr2_virtual_pad.h`  
`core/sbr2_virtual_pad.cpp`  
`test/sbr2_virtual_pad_test.cpp`  

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
# GGAI / SBR2 AI 開発 現在状況まとめ（更新版・2026/04/01）
# ============================================

## ■ このREADMEの役割（再確認）

この README は  
👉 **スレ移行しても100%続きから再開するための固定資産**

---

## ■ 現在位置（このスレ終了時点）

### 開発フェーズ
👉 **AIコア + 防御拡張 + level gate 実装完了 / 仮想コントローラー接続と最小入力確認は成功 / 次は AI と仮想入力層の接続フェーズ**

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

### 仮想コントローラー最小接続層
- `core/sbr2_virtual_pad.h` を追加済み
- `core/sbr2_virtual_pad.cpp` を追加済み
- `test/sbr2_virtual_pad_test.cpp` を追加済み

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

### ④ 仮想コントローラー接続の最小確認に成功（今回の最大成果）

#### C# 側（ScpDriverInterface）
- `PlugIn(1)` による仮想 Xbox 360 コントローラー接続成功
- `Report()` による入力送信成功
- `Unplug(1)` による切断成功

#### 確認した入力
- `neutral`
- `RIGHT`
- `A button`

#### OS 側で確認したこと
- `joy.cpl`（Windows の「ゲーム コントローラー」）で認識
- ボタン ON / OFF の反応確認
- 方向入力 ON / OFF の反応確認
- 接続音 / 切断音の確認

#### 結論
- **SCP Virtual Bus Driver を user-mode から操作可能**
- **仮想入力によるゲーム操作は実現可能**

---

### ⑤ C++ 側でも SCP 接続・入力送信の最小確認に成功

#### 新規追加ファイル
- `core/sbr2_virtual_pad.h`
- `core/sbr2_virtual_pad.cpp`
- `test/sbr2_virtual_pad_test.cpp`

#### 実装内容
- SetupDi + CreateFile によるデバイス取得
- DeviceIoControl による
  - `PlugIn`
  - `Report`
  - `Unplug`

#### C++ テスト結果
- `connect ok`
- `send_neutral ok`
- `send_right ok`
- `send_bomb ok`
- `release_all ok`
- `disconnect done`

#### 現在の位置づけ
- C++ 側の最小接続層は **最小動作確認成功**
- ただし **GGAI 本体（AI Brain / action 決定）とは未接続**

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

## ■ 仮想コントローラーに関する注意（重要）

- SCP は公式配布元が統一されていない
- 特定の zip 配布物に依存しない
- GGAI はドライバ本体を同梱しない
- 各自で導入する前提で進める

### 導入済みとみなす最低条件
- `PlugIn / Unplug` が成功する
- `joy.cpl` で仮想 Xbox 360 コントローラーが認識される
- ボタンまたは方向入力に反応がある

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
| 仮想コントローラー接続 | **C++ で最小動作確認済み（GGAI 未接続）** |

👉 **AIコア + 防御拡張はかなり前進。仮想コントローラー接続と最小入力確認も成功。次の最大の山は AI と仮想入力層の接続。**

---

## ■ 次にやること（次スレ開始点）

### 最優先
👉 **GGAI 用の仮想入力層を AI に接続する**

### 方針
amaAI の公開ソースから接続コードを拾う前提は一旦やめ、  
**GGAI 用に自作した最小接続層をそのまま育てる** 方針で進める。

### 最初の目標
1. `SBR2Action` を仮想入力へ変換する最小層を作る  
   - `WAIT` → `neutral`
   - `RIGHT` 系移動 → `RIGHT`
   - `PLACE_BOMB` 系 → `A button`
2. 最初は **固定入力ではなく、最小の action 変換** を通して入力送信する
3. そのあと AI Brain とつなぐ
4. 最後にゲーム上で最小確認する

### 仮想コントローラー接続で参照するもの
- `original_amaAI/`
- `reference_runtime/ama_v1_8_2/`
- `reference_runtime/scp_driver/`
- `core/sbr2_virtual_pad.h`
- `core/sbr2_virtual_pad.cpp`
- `test/sbr2_virtual_pad_test.cpp`

### ただし現時点の判断
- 公開されている `original_amaAI` からは、接続コードが見つからない可能性が高い
- なので **参考資料としては参照するが、接続層そのものは自作済みのものを使って進める**

---

## ■ 開発方針（次スレ）
- 大規模変更禁止
- 小さい修正のみ
- 必ずテストで確認
- コピペ形式で指示
- エラーは最優先修正
- 仮想コントローラー接続は、まず **最小の action 変換確認** から始める

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
- `core/sbr2_virtual_pad.h`
- `core/sbr2_virtual_pad.cpp`
- `test/sbr2_ai_brain_test.cpp`
- `test/sbr2_virtual_pad_test.cpp`

参考資料:
- `original_amaAI/`
- `reference_runtime/ama_v1_8_2/`
- `reference_runtime/scp_driver/`

重要:
- いきなりコードを書かない
- まず現状整理
- 次にやることは **GGAI 用の仮想入力層を AI に接続すること**
- 小さい変更のみ
- コピペ形式で指示
- エラーは最優先修正

---

## ■ 現在の最重要ポイント（まとめ）

👉 AIコアはかなり進んだ  
👉 防御拡張もかなり進んだ  
👉 amaAI 公開ソースから接続コードをそのまま拾うのは難しそう  
👉 仮想コントローラー接続と最小入力確認は **C# / C++ とも成功済み**  
👉 次の最大の山は **AI と仮想入力層の接続**  
👉 まずは **SBR2Action → 仮想入力** の最小変換から進める

# ============================================

# ============================================
# 追加更新（2026/04/02 Vision・入力接続フェーズ）
# ============================================

## ■ 今スレでやったこと（追記）

### ⑥ Vision + 実画面キャプチャ導入（基礎完成）
- Windows API（BitBlt）で画面取得成功
- `[capture] ok` ログ確認済み
- 現在はダミーstate（phase/座標）

### ⑦ 仮想入力 + AI テスト接続
- `sbr2_ai_pad_test.cpp` にて AI → 仮想コントローラー接続
- 実ゲーム上で移動・爆弾設置確認済み

### ⑧ フェーズ制御（仮）
- READY / GO / RESULT をフレームで仮再現
- 実ゲームとは未同期（Vision未完成）

### ⑨ GO中挙動の改善
- 爆弾設置後のもたつき解消（即再判断）
- GO中1回だけ爆弾設置フラグ導入
- 定数化：
  - kBombDecisionFrame
  - kImmediateEscapeFramesAfterBomb
  - kPostBombEscapeFrames

### ⑩ 現在の問題
- phaseがダミー
- 座標がダミー
- 挙動はテスト用

---

## ■ 現在の到達地点

👉 AI → 仮想入力 → ゲーム操作 が成立  
👉 画面取得も成功  

残りは  
👉 Vision（認識）  
👉 AI判断の本格接続  

---

## ■ 次にやること

### 最優先
- READY / GO の画像認識

### 次
- 自分・敵座標取得

### その後
- ダミーphase削除
- AI Brain接続

---

## ■ 状態まとめ

| 項目 | 状態 |
|---|---|
| AIコア | 完成 |
| 仮想入力 | 完成 |
| 実ゲーム操作 | OK |
| Vision取得 | OK |
| Vision認識 | 未実装 |

👉 次フェーズ：Vision実装

# ============================================
# 追加更新（2026/04/03-04 Phase認識仕上げ + A前提固定フェーズ）
# ============================================

## ■ このスレで固定したテスト前提（最重要）

このスレ以降、Vision の phase テストは **A前提** に固定する。

### A前提の流れ
1. DRAW直前でポーズ開始  
2. 仮想コン接続  
3. ゲーム画面でポーズ解除  
4. すぐに DRAW になる  
5. オレンジと黄色の帯によるラウンド遷移演出が入る  
6. 次ラウンド開始で READY が出る  
7. READY が消えて GO が出る  
8. しばらく待って決着し、DRAW か WINNER が出る  

👉 今後この README を読んで次スレへ移行する場合も、  
Vision の phase ログは **A前提** として読むこと。

---

## ■ 今スレでやったこと（追記）

### ⑪ Vision provider まわりの主要ファイル整理
今回の phase 認識で重要だったファイルは以下。

- `core/sbr2_game_state.h`
- `core/sbr2_game_state_provider.h`
- `core/sbr2_vision_game_state_provider.h`
- `core/sbr2_vision_game_state_provider.cpp`
- `core/sbr2_screen_capture.h`
- `core/sbr2_screen_capture.cpp`
- `test/sbr2_ai_pad_test.cpp`

---

### ⑫ 画面キャプチャをゲームウィンドウ基準に整理
- `sbr2_screen_capture.cpp` で SBR2 ウィンドウ矩形を基準に BitBlt 取得する方式を採用
- `mode=sbr2_window_blt` ログ確認済み
- `PrintWindow` は白画面化したため不採用
- 現在は **ゲームウィンドウ矩形をデスクトップから切り出す方式**

#### 現時点の注意
- 前面に別ウィンドウが重なると混ざる可能性あり
- ただし READY / GO / DRAW / WINNER の認識には実用域

---

### ⑬ READY / GO / RESULT 系 probe を追加
`core/sbr2_vision_game_state_provider.cpp` にて、画像中の矩形から色条件付き画素数を数える probe を追加した。

#### READY probe
- `ready_probe`
- 中央付近の READY 表示を白 / 暖色系で観測
- `ready_visible_streak`
- `ready_hidden_streak`
- `ready_active`

#### GO probe
- `go_probe`
- 中央付近の GO 表示を主に緑 / 白系で観測
- `go_visible_streak`
- `go_hidden_streak`
- `go_active`

#### RESULT 系 probe
- `winner_probe`
  - 現在は実質的に **WINNER専用ではなく RESULT 表示寄りの probe**
- `draw_probe`
  - DRAW 用の白 / 銀系 probe
- `result_visible_streak`
- `result_hidden_streak`
- `result_active`
- `result_gate`
  - READY/GO active 中は RESULT 判定しない
- `result_band`
  - オレンジ / 黄色の帯演出を検出し、そのフレームの RESULT probe を無効化
- `winner_right_noise`
  - 右側だけ極端に高いノイズを除外するガード

---

### ⑭ phase 判定をフレーム数ベースの active 制御へ変更
単なる visible 判定ではなく、  
**何フレーム連続で見えたか / 何フレーム連続で見えなくなったか**
で ON/OFF する方式へ変更した。

#### 現在の active 条件
- READY
  - 3フレーム見えたら ON
  - 2フレーム見えなくなったら OFF
- GO
  - 2フレーム見えたら ON
  - 2フレーム見えなくなったら OFF
- RESULT
  - 4フレーム見えたら ON
  - 1フレーム見えなくなったら OFF

#### 現在の phase 優先順
1. `result_active` → `RESULT`
2. `go_active` → `GO`
3. `ready_active` → `READY`
4. それ以外 → `detect_phase(tick)` fallback

#### fallback の現在値
- `detect_phase(tick)` は **常に GO を返す**
- ダミー READY / ダミー RESULT は削除済み
- つまり実質的な phase 本体は **Vision の active 判定**

---

### ⑮ READY / GO / WINNER / DRAW 認識の到達点
A前提ログに基づく現時点の評価。

#### READY
- **かなり認識できている**
- 帯演出後に `ready_active=1` で READY に戻るログ確認済み

#### GO
- **かなり認識できている**
- `go_active=1` で GO に移るログ確認済み

#### DRAW
- **認識できている**
- 接続直後の DRAW 区間で `draw_probe confirmed=1`、`phase=RESULT` 確認済み

#### WINNER
- **認識できている寄り**
- 後半の結果表示で `winner_probe/draw_probe` が強く立ち、`phase=RESULT` 確認済み

#### 結論
👉 **READY / GO / WINNER / DRAW は、現時点でかなり実用ライン**

ただし完全無欠ではなく、以下の残課題あり。

---

### ⑯ RESULT 検出の残課題
#### まだ残る問題
- 右側ノイズで `winner_total` / `winner_right_count` がやや高く出る区間がある
- ただし現在は `winner_visible=0` に落ち、`result_gate raw=0 active=0` で phase を壊さないところまで改善済み
- RESULT probe の生値はまだ完璧ではないが、**phase としてはかなり安定化した**

#### 今スレで入れた対策
- `result_band_active`
  - オレンジ / 黄色の帯演出を検出したら `winner_visible=false` / `draw_visible=false`
- `winner_right_noise`
  - 右側だけ異常に高いとき `winner_visible=false`

---

### ⑰ A前提の最初の先走り対策
`test/sbr2_ai_pad_test.cpp` にて、  
**最初の RESULT を一度見るまでは通常AIを動かさない** 制御を追加。

#### 追加変数
- `result_seen_once`

#### 現在の挙動
- 接続直後の曖昧区間では WAIT 寄りに抑える
- 一度 DRAW/RESULT を見たあとから次ラウンド READY / GO を通常処理

※ ただし最初の 1 回だけ完全には still に抑え切れていないログもあったため、ここは将来的に Connect/Disconnect UI や「READY を見てから操作開始」でさらに改善余地あり。

---

### ⑱ GO 開幕ボムの短い遅延制御を追加
`core/sbr2_game_state.h` と `core/sbr2_vision_game_state_provider.cpp`、`test/sbr2_ai_pad_test.cpp` にて、  
GO 開幕で即ボムを置くタイミングを制御するためのフラグを追加した。

#### 追加済み
- `go_open_delay_active`

#### 意味
- GO を見たあと、短い遅延を設ける
- 遅延が切れた瞬間から開幕ボムを置ける
- phase の安定と開幕ボムの強さを分離して調整する土台

#### 重要
これは **常時の爆弾配置許可** ではなく、  
**GO 開幕の短い特別処理** として扱う。

---

### ⑲ 現時点での到達点（このスレ終了時点）

| 項目 | 状態 |
|---|---|
| 画面キャプチャ | 実用域 |
| READY 認識 | かなり良い |
| GO 認識 | かなり良い |
| DRAW 認識 | かなり良い |
| WINNER 認識 | かなり良い |
| RESULT 帯演出除外 | 実装済み |
| RESULT 右側ノイズ抑制 | 実装済み（微調整余地あり） |
| GO 開幕遅延フラグ | 実装済み |
| A前提最初の先走り抑制 | 実装済み（なお改善余地あり） |

👉 **phase 認識フェーズはかなり仕上がった。次フェーズへ進める土台はできている。**

---

## ■ 次スレで最優先にやること（更新）

### 1. 接続管理の整理
- 仮想コン接続してから **認識だけ開始**
- ゲーム画面で **READY を見てから操作開始** に整理したい
- amaAI のような `Connect` / `Disconnect` UI も将来的にほしい

### 2. 次の主フェーズ
次スレでは以下へ進む。

- 自機座標（AI側なので `self`）取得
- 敵座標（プレイヤー側なので `enemy`）取得
- 開幕位置関係からの行動判断
- ダミー座標削除
- AI Brain への本接続整理

### 3. 次スレ開始時の重要前提
- このスレの Vision phase テストは **A前提固定**
- 次スレでも、過去のログを読むときはその前提で読むこと
- ただし実装主題は phase 認識から **座標取得フェーズ** へ移す

---

## ■ 次スレ開始テンプレ（更新版）

このプロジェクトの続きです。  
README.md とコードを読んで現状整理してください。

GitHub:  
https://github.com/guyat/ggai-bomberman-ai

作業ディレクトリ:  
`/c/Users/PC_User/Documents/GGAI/ggai`

まず読むファイル:
- `README.md`
- `core/sbr2_game_state.h`
- `core/sbr2_game_state_provider.h`
- `core/sbr2_vision_game_state_provider.h`
- `core/sbr2_vision_game_state_provider.cpp`
- `core/sbr2_screen_capture.h`
- `core/sbr2_screen_capture.cpp`
- `test/sbr2_ai_pad_test.cpp`
- `core/sbr2_ai_brain.cpp`
- `core/sbr2_ai_brain.h`
- `test/sbr2_ai_brain_test.cpp`

重要:
- phase テストは **A前提固定**
  - DRAW直前ポーズ開始
  - 仮想コン接続
  - ポーズ解除
  - DRAW
  - 帯演出
  - READY
  - GO
  - 決着して DRAW/WINNER
- いきなりコードを書かない
- まず現状整理
- 次にやることは **self / enemy 座標取得フェーズへ進むこと**
- 小さい変更のみ
- コピペ形式で指示
- エラーは最優先修正

---

## ■ 現在の最重要ポイント（更新版まとめ）

👉 AIコアはかなり進んだ  
👉 仮想入力も実ゲーム操作も成立済み  
👉 Vision 画面取得も成立済み  
👉 READY / GO / WINNER / DRAW の認識はかなり実用ライン  
👉 A前提のテスト運用を固定した  
👉 次の最大の山は **self / enemy 座標取得と AI Brain 本接続**  
👉 その前段として、将来的には **Connect / Disconnect UI と「READY を見てから操作開始」** を入れるとさらに運用が楽になる

# ============================================
