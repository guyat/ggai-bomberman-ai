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

## 現在のAI実装状況（2026年3月20日現在）

### 実装済みコア

- Bomb simulator
- DangerMap（未来爆風予測）
- Time-aware BFS
- Safe cell search
- Escape route reconstruction
- First action extraction
- AI Brain
- Future danger escape AI

### 実装済み攻撃AI

- 爆弾を置いた後に逃げ切れるかの判定
- 直線キル判定（straight-line kill）
- 1歩先敵位置予測による攻撃
- Trap / 閉じ込めの基礎
- Tricky用の trap 優先ロジック
- second chance trap（Tricky高Lv）
- 近距離ゴリ押し（Aggressive寄り）
- 近距離詰ませチェック
- 完全詰み判定（checkmate）
- 誘導トラップ（guided_trap / guided_trap_v2）
- survivable cell を使った trap / checkmate 系判断

### 実装済みレベル差

- Lv1〜Lv20 の土台あり
- レベルにより以下が変化する
  - 直線キルの解禁
  - 1歩先予測の解禁
  - trap の解禁
  - クールダウン差
  - trap 閾値差

### 実装済みスタイル差

- `Aggressive`
  - 爆弾を置きやすい
  - 多少強引
- `Careful`
  - 安全重視
  - 危険なら攻撃せず回避しやすい
- `Tricky`
  - trap寄り
  - 読み合い寄り
  - 高Lvで trap 優先

### 実装済み「人間っぽさ」改善

- 危険でない時の軽い再配置
- WAITしすぎない
- 壁に向かうだけの移動を減らす
- 進めない方向へ脳死で突っ込まないよう改善
- 同方向への無駄な寄りすぎを少し抑える処理
- reposition hold
- 同方向連打の抑制

---

## 現在できること（実態ベース）

- 未来の爆風を予測できる
- 危険が近いなら逃げられる
- 時間付き BFS により安全マスへの経路探索ができる
- 経路から最初の行動を取り出せる
- 爆弾を置いた後、自分が逃げ切れるかを判定できる
- 敵が直線上にいる場合に攻撃判断できる
- 1手先の敵位置を少し読んで攻撃できる
- trap の基礎判断ができる
- guided trap 系判断ができる
- survivable cells を使った詰ませ寄り判断ができる
- Lv差がある
- スタイル差がある
- 少しずつ人間っぽい再配置をする

---

## 現在のテスト解釈（現仕様）

### 注意

テストコードの表示文の中には、過去仕様の名残で `WAIT` 前提だったものがある。  
現在は「安全なら軽く位置調整する」仕様が入っているため、

- `WAIT`
- `UP`
- `LEFT`
- `RIGHT`
- `DOWN`

のどれかになっても、それが **仕様どおり** のケースがある。

### 現在の見方

- CASE4: safe start
  - WAITでなく再配置でもOK
- CASE6: blocked straight-line
  - 大事なのは `PLACE_BOMB` しないこと
- CASE8: note-only case
  - trap未保証盤面なので再配置もOK
- CASE9: Lv5
  - 低Lvらしく、直線キルしないことが重要
- CASE10: Lv20
  - 高Lvらしく直線キルすることが重要
- CASE12: Careful
  - recklessな爆弾設置を避けることが重要
  - `UP` など回避行動でも成功扱い
- CASE13: Tricky / trap-oriented
  - 現時点では `guided_trap_v2` になっている
- CASE14: Tricky trap test
  - 現時点では `guided_trap_v2` になっている
  - この挙動を維持しつつ、強すぎないかを今後確認する
- CASE15〜17
  - 表示簡略化済み
  - `OK` / `NOTE observation only` を見る

---

## このスレ終了時点の最新状況（最重要）

### ビルド状態

- ビルドは成功している
- 直近の状態ではコンパイルエラーは出ていない

### テストの現状

- CASE10〜17 は通っている
- CASE13 は `guided_trap_v2`
- CASE14 は `guided_trap_v2`
- CASE13 / CASE14 を `guided_trap_v2` に寄せる調整は完了した
- 次スレの主題は、guided_trap_v2 が強すぎないかの確認と、次にどの調整へ進むかの整理

### このスレでやった重要なこと

#### 追加・調整したもの

- `guided_trap_v2` 関連の条件調整
- `trap_like` 条件の見直し
- `checkmate` / `guided_trap` / `guided_trap_v2` の検討
- 近距離 pressure / close_range_checkmate 周りの追加

#### 削除したもの

- `goto` を使う straight_kill 抑制案
- `SKIP_STRAIGHT_KILL` ラベル案
- 一時的に入れた `std::cout` デバッグ出力

#### 現在の `trap_like` 条件

現時点では以下になっている前提で再開すること。
CASE13 / CASE14 を guided_trap_v2 に寄せた後の最新版である。

```cpp
bool trap_like =
    !escape1_ok ||
    escape1_danger ||
    !escape2_ok ||
    escape2_danger ||
    (escape1_ok && escape2_ok);
```

---

## 次スレで最優先でやること

### 目的

CASE13 / CASE14 を `guided_trap_v2` に寄せる調整は完了した。  
次は、この `guided_trap_v2` 条件が強すぎないかを確認し、必要なら小さく絞る。

### 重要

この段階は復旧ではなく前進フェーズ。  
大きな構造変更はせず、小さな条件調整とテスト確認を優先する。

---

## 次スレでの進め方（絶対守る）

### 1. いきなりコードを書かない

最初に必ずやること：

* README.md を読む
* `core/sbr2_ai_brain.cpp` を読む
* `core/sbr2_ai_brain.h` を読む
* `core/sbr2_board.h` を読む
* `test/sbr2_ai_brain_test.cpp` を読む

### 2. 最初に現状整理を書く

次スレで最初に assistant は必ず以下を行うこと。

* 現状のビルド状態を整理
* CASE13 / CASE14 の現在結果を整理
* CASE13 / CASE14 で `guided_trap_v2` が選ばれている現状を整理し、次にどこを調整するかを説明
* これからやる変更が

  * 復旧なのか
  * 前進なのか
    を最初に明言する

### 3. 小さい変更だけ行う

* 一度に大きく変更しない
* `goto` を使わない
* ラベルジャンプを使わない
* まずは if 条件や分岐順などの小変更を優先する

### 4. エラーが出たら最優先で直す

* 新機能追加よりエラー修正を優先
* エラーが出たら、まず原因説明 → 修正案 → ビルド確認の順で進める

### 5. CASE17 を壊さない

* 過去に壊したことがあるので、広範囲変更は禁止
* 変更箇所は必ず限定する

---

## ユーザー前提（毎回守る）

開発者（ユーザー）はプログラミングに不慣れである。
そのため、作業指示では以下を必須とする。

* どのファイルを直すか明示する
* 丸ごと置き換えか一部修正か明示する
* 「この文字列を検索してください」と案内する
* コピペしやすい形で出す
* 変更前と変更後を明示する
* 「どこからどこまで消すか」を明示する
* 曖昧な指示は禁止する

---

## 現在位置（フェーズ整理）

README下部の古いフェーズ表では
「Phase 2: 爆弾設置AI 実装中」
などの古い表現が残っている箇所があるが、現状はそれよりかなり進んでいる。

現時点の実装位置は概ね以下。

### 完了済みに近いもの

* Phase 1: 基本回避AI
* Phase 2: 爆弾設置AIの基礎
* Phase 3: 攻撃判断AIの基礎
* Phase 4: Trap AI の基礎
* Lv差 / Style差の土台

### 現在やっていること

* Tricky の trap 精度改善
* guided trap の条件調整
* 人間っぽい動きの改善
* 位置調整ロジックの改善
* 動きの不自然さを減らす

---

## 重要ゲーム仕様（固定）

### ラウンド開始

* READY 中は移動可能
* READY 中は爆弾を置けない
* GO! が出てから爆弾設置可能

### 爆弾

* 爆弾保有数 = 8 固定
* 火力 = 8 固定
* 爆弾設置から **153F** で爆発
* 誘爆は **10F後**
* 爆風は射程内に一気に出る
* 爆風は同一フレームで全部反映

### ステージ

* 11 × 13
* 外周は壁
* 固定ハードブロックあり
* ソフトブロックなし

---

## 移動仕様（重要）

### 現在把握済み

* center → next = 6F
* normal move = 11F

### 現状

PathFinder などの一部では、簡略化のため `MOVE_FRAMES = 11` を使っている箇所がある。

### 今後の重要仕様

SBR2 では、

* **斜め入力しながらハードブロック等を曲がると**
* **垂直移動だけより少し早めにコーナリングできる**

という仕様がある。

これは今後、

* 経路探索
* 到達フレーム計算
* 回避精度
* 実機らしい移動
* 仮想コントローラー精度

に大きく影響するため、
**「移動モデルの一部」として管理する予定**。

この仕様は後から動画検証ベースで精密実装する。

---

## 上級テクニック方針

以下は後で実装する予定。

* Bomb Tail
* Throw Chain
* Punch Chain
* Kick Chain
* Throw Death Size
* Punch Death Size
* Kick Death Size
* Exploding Punch
* Right Timing Exploding Punch

### 方針

各テクニックは **動画によるフレーム検証後に実装** する。
今はまだやらない。

特に、

* 起爆パンチ
* ボムテイル
* デスサイズ系
* 誘爆技群

は、動画で仕様を確認してから実装する。

---

## 次スレで最初に貼る文言（そのまま使う）

このプロジェクトの続きです。
まず README.md と現在のコードを必ず読んでから始めてください。

GitHub:
[https://github.com/guyat/ggai-bomberman-ai](https://github.com/guyat/ggai-bomberman-ai)

作業ディレクトリ:
`/c/Users/PC_User/Documents/GGAI/ggai`

特に読むファイル:

* `core/sbr2_ai_brain.cpp`
* `core/sbr2_ai_brain.h`
* `core/sbr2_board.h`
* `test/sbr2_ai_brain_test.cpp`

重要:

* 先走らず、まず現状整理をしてください
* 変更前に「復旧なのか前進なのか」を明言してください
* どのファイルのどこをどう直すか明示してください
* 丸ごと置き換えか一部修正か明示してください
* コピペしやすいコード提示をしてください
* エラーが出たら最優先でその原因説明と復旧をしてください
* goto や大きな構造変更は使わないでください

現在の主題:

* CASE13 / CASE14 を `guided_trap_v2` に寄せる調整は完了
* 次は `guided_trap_v2` の条件が強すぎないか確認したい

このREADMEと現在コードを読んだうえで、現状整理から始めてください。

```