# GGAI Bomberman AI

こちらのGitHubからコードを読み取ってください。
Repository:
https://github.com/guyat/ggai-bomberman-ai

Steam版（PC版）**Super Bomberman R2 (SBR2)** の **1vs1 用 AI** を開発するプロジェクトです。

このAIは **SCP Virtual Bus Driver（scp_driver）による仮想コントローラー入力**でゲームを操作することを前提としています。

- 対象ゲーム: Steam版 Super Bomberman R2
- 対象ルール: 1vs1
- 目的: フレーム単位の未来予測を使って、**高精度な回避・爆弾設置・将来的にはトラップまで行う強AI** を作る

---

# 開発方針

開発者（ユーザー）はプログラミングに不慣れなため、作業指示は以下を強く重視します。

- できるだけ丁寧に説明する
- どのファイルをどう直すか明確にする
- コピペで丸写ししやすい形で提示する
- 「丸ごと置き換え」か「一部追記」かを必ず明示する

また、長い会話ログだけに依存すると重要仕様が抜け落ちることがあるため、  
**重要なゲーム仕様・AI前提は README.md に明文化して固定資産化する** 方針とします。

---

# 現在のAI実装状況

## 実装済み

- Bomb simulator
- DangerMap（未来爆風予測）
- Time-aware BFS
- Safe cell search
- Escape route reconstruction
- First action extraction
- AI Brain
- Future danger escape AI

## 現在できること

- 未来の爆風を予測できる
- 近い未来で危険になるマスを避けられる
- 時間付き BFS により、安全マスへの経路探索ができる
- 経路から最初の行動を取り出せる
- 「危険が近いなら逃げる」という判断ができる

## 次の実装目標

- 爆弾設置AI
- 設置後に自分が逃げ切れるかのシミュレーション
- 敵に当たる爆弾のみ置くロジック
- トラップ / 閉じ込め
- CPUレベル差（Lv1～Lv20）設計

---

# リポジトリ構成

```text
ggai/
├─ core/
│  ├─ sbr2_ai_brain.cpp
│  ├─ sbr2_ai_brain.h
│  ├─ sbr2_board.h
│  ├─ sbr2_bomb.h
│  ├─ sbr2_pathfinder.cpp
│  ├─ sbr2_pathfinder.h
│  ├─ sbr2_simulator.cpp
│  └─ sbr2_simulator.h
├─ test/
│  ├─ sbr2_simulator_test.cpp
│  ├─ sbr2_pathfinder_test.cpp
│  └─ sbr2_ai_brain_test.cpp
├─ README.md
└─ .gitignore
````

---

# 開発環境

* OS: Windows
* Shell: Git Bash (MINGW64)
* Compiler: g++ (C++17)
* Working Directory: `/c/Users/PC_User/Documents/GGAI/ggai`

---

# ゲーム仕様（重要）

## 対象ゲーム

* Steam版 Super Bomberman R2
* 1vs1

## ステージ

* 盤面サイズ: **11 × 13**
* 外周は壁
* 固定ハードブロックあり
* ソフトブロックなし
* プレイヤーはハードブロック上には立てない

## 爆弾性能

* 爆弾保有数: **8**（最大値固定）
* 火力: **8**（最大値固定）
* 爆風は一気に伸びる（途中フレームで1マスずつではない）
* 爆風の到達は **同一フレーム内で一斉反映**
* 爆風射程は最大 8 マス

---

# ラウンド開始仕様（重要）

SBR2 のラウンド開始時は以下の流れです。

```text
READY 表示
↓
この間は移動可能
↓
GO! 表示
↓
爆弾設置可能になる
```

つまり、

* **READY 中は移動できる**
* **READY 中は爆弾を置けない**
* **GO! が出てから爆弾を置ける**

AI はこの仕様を尊重する必要があります。

---

# フレーム計測の前提

すべてのタイミングは、**60FPS の配信アーカイブを1フレームずつコマ送りで解析**して得た値を基準にします。

---

# 爆弾と爆風のフレーム仕様

## 通常爆発

* 爆弾を置いたフレームを **0F** とする
* **153F で爆発**

## 誘爆（Chain）

* ある爆弾の爆風が別の爆弾に当たった場合、
* **10フレーム後に誘爆側の爆弾が爆発**

例:

```text
A が爆発
↓
A の爆風が B に当たる
↓
10F 後に B が爆発
↓
さらに B の爆風が C に当たれば、その 10F 後に C が爆発
```

## 爆風の広がり方

* 爆風は徐々に1マスずつ伸びるのではなく、**射程内に一気に出る**
* 射程内の爆風は **同じフレームで全部出る**
* 火力 8 の場合、最大 8 マス先まで同一フレームで危険化する

---

# プレイヤー移動仕様（超重要）

## 基本方針

プレイヤー移動は **マスの中心から移動開始するケース** と、
**通常の連続移動中のケース** を分けて扱う必要があります。

これを落とすと、回避AIや経路探索の正しさが崩れます。

---

## 1. マス中央から移動開始した場合

```text
0F  移動開始
1F
2F
3F
4F
5F  まだ元のマスにいる扱い
6F  次のマスに到達
```

つまり:

* **center → next = 6F**

---

## 2. 通常の連続移動

1マス進んで、すでに次マスに到達した後の連続移動は以下。

```text
0F  現在マスに到達したフレーム
1F
2F
3F
4F
5F
6F
7F
8F
9F
10F  まだそのマスにいる扱い
11F  次のマスに到達
```

つまり:

* **normal move = 11F / 1マス**

---

## 現在実装上の簡略化

現時点の PathFinder は、実装簡略化のため

* **MOVE_FRAMES = 11**

で統一している箇所があります。

これは将来的には改善対象です。
理想的には以下を分けて扱う必要があります。

* 最初の1歩: 6F
* 連続移動: 11F

---

# DangerMap 仕様

* 未来予測範囲: **512F**
* DangerMap は、各フレーム・各マスが危険かどうかを保持する
* DangerMap は爆弾・誘爆・爆風をすべて考慮して生成される
* Pathfinder / Escape AI はこの DangerMap を参照して行動を決める

---

# AI設計の基本思想

## 現在のパイプライン

```text
DangerMap
↓
Time-aware BFS
↓
Safe cell detection
↓
Escape route reconstruction
↓
AI Brain
```

## 現在の優先順位

現時点のAIは **攻撃より生存優先** です。

基本思想:

1. 今後危険になるかを予測
2. 危険なら逃げる
3. 安全なら待機または次の攻撃判断へ進む

---

# テスト前提

再現性確保のため、開発中のテストでは開始位置を固定する。

* AI = 右下固定
* 相手 = 左上固定

ただし、本番仕様は固定ではなく、

* **4隅ランダム開始**

です。

したがって、テストコードの固定開始位置は
**再現性のための仮条件** であり、本番ロジックの前提にしてはいけません。

---

# 現在のテスト状況

確認済みの代表ケース:

* CASE1: safe start → WAIT
* CASE2: danger soon → ESCAPE成功
* CASE3: Pathfinder と Brain が一致

今後、爆弾設置AI実装後は以下のようなケースを増やす。

* 安全時に PLACE_BOMB を返すか
* 置いた後に逃げ切れないなら PLACE_BOMB しないか
* 近い未来で危険なら設置より回避を優先するか
* 誘爆込みで安全判定できるか

---

# 次に実装する仕様：爆弾設置AI

## 目標

AI が爆弾を置く前に、

* その場に今置いた場合の未来
* 誘爆込みの未来
* 置いた後、自分が逃げ切れるか

をシミュレーションし、**逃げ切れる場合のみ爆弾を置く**。

## 最低限の設置条件

少なくとも以下を満たす必要がある。

1. 現在マスが危険すぎない
2. その場に爆弾を仮置きした未来を再シミュレーションできる
3. 爆発まで・または爆発終了まで生き残れる逃走経路がある
4. 逃げられない置き方はしない

## 将来の強化

* 敵に当たる時だけ置く
* 相手の逃げ道を減らすように置く
* トラップ化する置き方を優先
* リスクとリターンで設置判断を調整

---

# 将来的に扱いたいアクション仕様

## パンチ

会話中に確認された特殊操作として **パンチ** がある。

把握している内容:

* パンチすると爆弾が移動する
* もともとあった場所から **3マス先** へ飛ぶ / 移動する
* 着弾後、**すぐ爆発**

## 起爆パンチ

さらに高度なテクニックとして **起爆パンチ** がある。

把握している内容:

* 誘爆までの10Fの途中（会話上では「5Fくらい」と表現）でパンチ入力すると起こる特殊挙動
* かなり難しいテクニック
* 現時点では優先度を下げ、まずは通常の回避・設置AIを先に完成させる

### 注意

このパンチ関連仕様は、まだ実装仕様として固め切っていない部分があるため、
実装前に再検証すること。

---

# 実装上の注意点

## 会話ログだけを信用しない

このプロジェクトでは、長い会話や環境差によって重要仕様が抜ける可能性がある。
そのため、最終判断は以下を基準とする。

1. README.md
2. 現在のローカルコード
3. テスト結果

## 特に落としてはいけない重要仕様

* READY 中は移動できるが爆弾は置けない
* GO 後に爆弾設置可能
* 爆弾 153F
* 誘爆 10F
* center → next = 6F
* normal move = 11F
* DangerMap = 512F
* 火力 8
* 爆弾保有数 8
* ステージ 11 × 13
* ソフトブロックなし
* 本番は4隅ランダム開始

---

# CPUレベル構想（将来）

例:

* Lv1: ほぼランダム移動
* Lv5: 目先の爆風を避ける
* Lv10: future danger escape
* Lv15: 置き逃げ可能な爆弾設置
* Lv20: 予測・誘導・トラップまで行う高難度AI

---

# 長期目標

最終的には、以下を備えた **高レベルなボンバーマンCPU AI** を目指す。

* 未来爆風予測
* 時間付き経路探索
* 安全な回避
* 爆弾設置
* 敵追い詰め
* トラップ
* レベル別難易度制御

---

# 要確認 / 再検証メモ

以下は会話中に出た内容だが、実装に入る前に再検証したい項目。

* パンチ / 起爆パンチの厳密フレーム
* READY 中の移動可否の細部
* 爆風持続フレームの厳密定義（現在実装定数との整合）
* 盤面座標系の最終表現（11×13 の扱いと外周壁の内部表現）
* center 6F / normal 11F を PathFinder にどう落とし込むか

---

# AI Architecture

## 全体構造

Game State
│
▼
Bomb Simulator
│
▼
DangerMap (future 512F)
│
▼
PathFinder (Time-aware BFS)
│
▼
Escape Route Reconstruction
│
▼
EscapeMap
│
▼
Enemy Future Map
│
▼
Position Evaluation Map
│
▼
KillMap
│
▼
AI Decision System
│
▼
AI State Machine
│
▼
Controller Input (scp_driver)
- 
- 日本語表記
- Game情報
- ↓
- 爆弾未来シミュレーション
- ↓
- 危険マップ
- ↓
- 時間付き経路探索
- ↓
- 回避経路復元
- ↓
- 敵逃走マップ
- ↓
- 敵未来予測
- ↓
- 位置評価
- ↓
- キル可能性
- ↓
- 意思決定
- ↓
- 状態制御
- ↓
- コントローラー入力
---

# CPU Level Design (Planned)

将来的に AI の強さを Lv1～Lv20 の段階で調整できるようにする計画。
各レベルは以下の要素で差をつける。

- Lv1～Lv20 のAI強度を段階的に調整可能にする
- 危険予測の深さ
- 回避精度
- 爆弾設置頻度
- 敵ヒット重視度
- トラップ使用有無
- 判断ミスの混ぜ方

---

---

# DangerMap Internal Structure

DangerMap は、未来 512F 分の各マスの危険状態を保持するための内部データ構造。

AI は DangerMap を使って、

・今いる場所が将来危険になるか  
・逃げ先が安全か  
・移動中に爆風に当たらないか  
・爆弾設置後に逃げ切れるか  

を判断する。

---

## 基本構造

DangerMap は概念的には次の配列。

danger[frame][y][x]

意味

frame = 未来フレーム  
y = 縦座標  
x = 横座標  

値

true  = 危険  
false = 安全

---

## サイズ

DangerMap の未来予測範囲

512F

盤面サイズ

11 × 13

つまり概念上は

danger[512][11][13]

---

## DangerMap生成の流れ

現在の爆弾状態  
↓  
各爆弾の爆発フレーム計算  
↓  
誘爆処理 (10F)  
↓  
爆風範囲計算 (火力8)  
↓  
各フレームの危険マスを記録  
↓  
DangerMap完成

---

## DangerMap参照例

AI は次のように DangerMap を参照する。

今のマスが安全か

danger[current_frame][y][x]

1フレーム後も安全か

danger[current_frame + 1][y][x]

移動先到達時に安全か

danger[arrive_frame][ny][nx]

---

## Time-aware BFSとの関係

PathFinder の状態は

(x, y, frame)

つまり

どこに  
いついるか  

を同時に探索する。

DangerMap はその安全判定を行う。

---

## 移動判定との関係

移動仕様

center → next = 6F  
normal move = 11F

現在の実装では簡略化して

MOVE = 11F

を使っている部分がある。

将来的には

最初の1歩 = 6F  
通常移動 = 11F  

を正確に扱う予定。

---

## 爆弾設置AIとの関係

爆弾設置AIでは

1. 仮に爆弾を置く  
2. 未来を再シミュレーション  
3. 新しいDangerMap生成  
4. 逃げ切れるかPathFinderで確認  

を行う。

---

## 要点

DangerMap はこのAIの中核データ。

用途

・未来危険予測  
・安全経路探索  
・回避AI  
・爆弾設置後の生存判定

一言で言うと

「未来の各フレームの危険マス表」


---

# Advanced Techniques (Planned)

将来的に、Steam版 Super Bomberman R2 の上級テクニックも AI に実装していくことを目標とする。

ただし、現時点では通常の回避・爆弾設置・安全確認を優先し、  
以下のテクニックは「将来実装予定の候補」として整理する。

また、ここに書く順番や分類は仮のものであり、  
後から順番変更・追加・細分類を行ってよいものとする。

---

## 実装候補テクニック（現時点の想定順）

難易度が比較的低いと思われるものから順に並べる。

1. Bomb Tail（ボムテイル）
2. Throw Chain（投げ誘爆）
3. Punch Chain（パンチ誘爆）
4. Kick Chain（蹴り誘爆）
5. Throw Death Size（投げデスサイズ）
6. Punch Death Size（パンチデスサイズ）
7. Kick Death Size（蹴りデスサイズ）
8. Exploding Punch（起爆パンチ）
9. Right Timing Exploding Punch（目押し起爆パンチ）

---

## 1. Bomb Tail（ボムテイル）

概要:

- 逃げながら 1マスごと、または適切な間隔で爆弾を連続設置する
- 後方を危険地帯にして追撃を防ぐ
- 防御・通路封鎖・追撃拒否として使う

今後検証したい点:

- 最適な設置間隔
- 逃走ルートとの両立
- 相手の追撃阻止効果
- 自滅しない条件

---

## 2. Throw Chain（投げ誘爆）

概要:

- 投げた爆弾を爆風や誘爆に絡めて、連鎖爆発を発生させる技

今後検証したい点:

- 投げ爆弾の移動仕様
- 着地判定
- 誘爆成立タイミング
- 爆風との重なり方

---

## 3. Punch Chain（パンチ誘爆）

概要:

- パンチした爆弾を爆風や誘爆に絡めて連鎖爆発を発生させる技
- 高難度のタイミング技として扱う可能性がある

今後検証したい点:

- パンチ後の移動仕様
- 誘爆までのフレーム
- 連鎖の成立条件
- 成功条件と失敗条件

---

## 4. Kick Chain（蹴り誘爆）

概要:

- キックした爆弾を爆風や誘爆に絡めて連鎖爆発を発生させる技
- 他の上級テクニックの土台になることが多い重要技術

現時点で把握している派生 / 種類:

- 通路蹴り誘爆
- 十字路蹴り誘爆

### 通路蹴り誘爆

概要:

- 爆風が出ているマスの1マス前に爆弾を置き、蹴って誘爆させる技

要検証:

- 正確な設置位置
- キック入力タイミング
- 成立しやすい通路条件
- 爆風との接触フレーム

### 十字路蹴り誘爆

概要:

- 爆風が出ているマスの2マス前に爆弾を置き、蹴って誘爆させる技

要検証:

- 十字路形状での成立条件
- 爆弾設置位置
- キック入力タイミング
- 誘爆成立フレーム

今後検証したい点:

- 爆風に触れた爆弾の挙動
- キックと誘爆のフレーム関係
- 派生技への接続条件
- 成立に必要な盤面条件

---

## 5. Throw Death Size（投げデスサイズ）

概要:

- 投げによって爆弾を移動させ、相手の逃げ道を塞いだり、致命的な爆風状況を作る技

今後検証したい点:

- 投げの移動仕様
- 着地点判定
- 爆発タイミングとの関係
- 誘爆込みの成立条件

---

## 6. Punch Death Size（パンチデスサイズ）

概要:

- パンチで爆弾を移動させ、相手の回避困難な状況を作る技
- 高度なタイミング技として扱う可能性がある

今後検証したい点:

- パンチ後の移動先
- 着弾後の挙動
- 誘爆との関係
- 成立条件と失敗条件

---

## 7. Kick Death Size（蹴りデスサイズ）

概要:

- キックで爆弾を動かし、相手に致命的な逃げづらい状況を作る技
- 蹴り誘爆を利用する派生が多く、重要度が高い

今後検証したい点:

- キック後の爆弾移動仕様
- 停止条件
- 誘爆との組み合わせ
- 成立しやすい盤面条件
- どの蹴り誘爆派生から接続できるか

---

## 8. Exploding Punch（起爆パンチ）

概要:

- 誘爆タイミングに合わせてパンチを入力し、特殊な起爆挙動を発生させる高難度テクニック

今後検証したい点:

- 入力受付フレーム
- 誘爆との厳密な関係
- 成立条件
- 失敗条件
- 通常パンチとの違い

---

## 9. Right Timing Exploding Punch（目押し起爆パンチ）

概要:

- 起爆パンチの中でも、さらに厳密なタイミング入力を要求する高難度技

今後検証したい点:

- 目押し入力の成立フレーム
- 通常の起爆パンチとの差
- 成功率に影響する条件
- AI実装時に必要な精度

---

## 場外・裏回り系テクニックについて

ボンバーマンでは、パンチや投げを場外方向へ行ったときに、  
爆弾が裏側へ飛んでいく / 回り込むような挙動を利用するテクニックが存在する。

これらも将来的には AI 実装候補に含める可能性がある。

ただし現時点では、以下の理由から個別実装対象としてはまだ早い段階とする。

- 通常のキック / 投げ / パンチ挙動の厳密仕様が未整理
- 蹴り誘爆などの基礎テクニックが先
- 動画による挙動確認が必要

そのため、まずは動画検証で仕様を固めた後に、  
必要であれば別章として追加する。

---

## 実装方針

これらの上級テクニックは、いきなり実装せず、以下の順番で進める。

1. 通常の安全回避  
2. 通常の安全爆弾設置  
3. 敵ヒットを狙う爆弾設置  
4. キック / 投げ / パンチの通常挙動  
5. 投げ誘爆 / パンチ誘爆 / 蹴り誘爆  
6. デスサイズ系  
7. ボムテイルや複合連携  
8. Exploding Punch / Right Timing Exploding Punch  
9. 場外・裏回り系の特殊テクニック  

---

## 動画検証について

これらのテクニックは、実際のゲーム動画を用いて仕様確認を行う予定。

動画から特に確認したい項目:

- 入力フレーム
- 爆弾移動フレーム
- 誘爆成立フレーム
- 技の成立条件
- 技の失敗条件
- 有効な盤面状況
- 派生技への接続条件

README にはまず概要を固定し、  
厳密なフレーム仕様や成立条件は今後の動画検証で追記していく。


---

## Level-based Unlock Design (Planned)

CPUレベルは、単純に反応速度や精度だけで差をつけるのではなく、  
「思考レベル」と「使用テクニックの解禁段階」によって強さを段階的に変える方針とする。

### 1. 思考レベルの段階

- 低Lv帯では、自分中心の判断を行う
  - 自分の安全確保
  - 単純な爆弾設置
  - 単純な回避

- 高Lv帯では、敵の行動予測を使う
  - 敵の逃げ先予測
  - 敵の進路妨害
  - 追い込み
  - 罠
  - 誘爆や上級テクニックとの連携

つまり、

- 低Lv = ② 自分中心のAI
- 高Lv = ① 敵の思考や逃げ先も読むAI

という形で段階的に強くする。

### 2. テクニック解禁の段階

レベルが上がるにつれて、使用するテクニックを段階的に解禁する。

例:

- Lv○○から Bomb Tail（ボムテイル）を使用開始
- Lv○○から Throw Chain（投げ誘爆）を使用開始
- Lv○○から Punch Chain（パンチ誘爆）を使用開始
- Lv○○から Kick Chain（蹴り誘爆）を使用開始
- Lv○○から Throw Death Size（投げデスサイズ）を使用開始
- Lv○○から Punch Death Size（パンチデスサイズ）を使用開始
- Lv○○から Kick Death Size（蹴りデスサイズ）を使用開始
- Lv○○から Exploding Punch（起爆パンチ）を使用開始
- Lv○○から Right Timing Exploding Punch（目押し起爆パンチ）を使用開始

### 3. 方針

各テクニックをどのLvから解禁するかは、今後の実装進捗・動画検証・実戦テストを見ながら調整する。

このため、Lvごとの解禁順や閾値は固定ではなく、後から変更可能な設計とする。

---

# AI Processing Flow

この AI は、現在のゲーム状態をもとに未来を予測し、  
安全性と攻撃可能性を見ながら次の行動を決定する。

## 全体フロー

```text
[Current Game State]
    │
    │  現在の情報を取得
    │  - 自分の位置
    │  - 相手の位置
    │  - 爆弾位置
    │  - 爆弾タイマー
    │  - 盤面情報
    ▼
[Bomb Simulator]
    │
    │  爆弾の未来をシミュレート
    │  - 爆発
    │  - 誘爆
    │  - 爆風
    ▼
[DangerMap]
    │
    │  各フレーム・各マスの危険状態を生成
    │  danger[frame][y][x]
    ▼
[PathFinder / Time-aware BFS]
    │
    │  時間付き探索
    │  状態: (x, y, frame)
    │
    │  - 安全マス探索
    │  - 逃走可能判定
    │  - 爆弾設置後に逃げ切れるか判定
    ▼
[Escape Route Reconstruction]
    │
    │  探索で見つかったルートから
    │  最初の1行動を復元
    ▼
[AI Brain]
    │
    │  状況に応じて行動を決定
    │
    │  例:
    │  - 近い未来で危険 → 逃げる
    │  - 安全で、置いてから逃げられる → PLACE_BOMB
    │  - 条件を満たさない → WAIT
    ▼
[Controller Output]
    │
    │  決定した行動を入力に変換
    ▼
[scp_driver / Virtual Controller Input]
```

---

# CPU Level Design (Draft)

CPUレベルは **Lv1〜Lv20** の段階で強くなる。

ただしこの表は **仮の設計** であり、  
テストプレイやAIの実装進捗に応じて **自由に調整・変更してよい**。

CPUの強さは以下の要素で段階的に上がる。

- 思考の賢さ
- 危険予測
- 敵行動の考慮
- 使用テクニック
- ミス率
- 先読み深さ

---

## CPU Level Table (Draft)

| Lv | 思考レベル | 主な行動 / 解禁要素 |
|----|-------------|---------------------|
| 1 | 回避のみ | 危険回避のみ |
| 2 | 回避 | 単純移動 |
| 3 | 回避 + 爆弾 | 安全なときだけ爆弾 |
| 4 | 回避 + 爆弾 | 単純な置き逃げ |
| 5 | 回避強化 | 逃げ判断が少し賢い |
| 6 | 攻撃開始 | 敵が近いと爆弾 |
| 7 | テクニック解禁 | Bomb Tail |
| 8 | 攻撃強化 | 置き逃げ判断改善 |
| 9 | 誘爆技 | Throw Chain |
| 10 | 誘爆技 | Punch Chain |
| 11 | 敵視点解禁 | 敵の逃げ先を少し考慮 |
| 12 | 誘爆強化 | Kick Chain |
| 13 | 敵予測 | 敵の逃げ道を意識 |
| 14 | 上級技 | Throw Death Size |
| 15 | 上級技 | Punch Death Size |
| 16 | 上級技 | Kick Death Size |
| 17 | 高難度技 | Exploding Punch |
| 18 | 高難度技 | Exploding Punch精度向上 |
| 19 | 超高難度技 | Right Timing Exploding Punch |
| 20 | 最強モード | 全テクニック + 敵予測最大 |

---

## 思考モードの段階

### Lv1〜Lv10

基本的に **自分中心のAI**

- 自分の安全確保
- 単純な爆弾設置
- 単純な回避

敵の行動はほぼ考慮しない。

---

### Lv11〜Lv16

**敵視点を少し導入**

- 敵の逃げ先を予測
- 進路妨害
- 誘爆技を使用

---

### Lv17〜Lv20

**高度な戦術AI**

- 上級テクニック
- 敵の逃げ道封鎖
- 複合戦術
- 高精度タイミング技

---

## 調整について

このCPUレベル設計は **暫定的なもの** であり、

- テクニック解禁Lv
- 敵視点導入Lv
- 思考の深さ
- ミス率

などは、今後のテストや動画検証を元に調整する。

将来的には

- Lvごとの思考設定
- 技使用確率
- 敵予測の強さ
- 先読みフレーム数

などを細かく設定できるようにする予定。

---

# Enemy Prediction Policy (Draft)

この AI は、将来的に **敵の行動予測** を使って強くしていく方針とする。

ただし、全Lvでいきなり「敵は常に最適行動をする」と仮定すると、  
低Lvでも不自然に強くなりすぎたり、人間味のない動きになりやすい。

そのため、CPUレベルに応じて  
**敵行動の予測方法そのものを段階的に変える** 設計を採用する。

---

## 基本方針

- 低Lvでは、敵の行動をランダム寄りに仮定する
- 中Lvでは、敵の行動を一部だけ読む
- 高Lvでは、敵が最適に逃げる前提で読む

つまり、

- 低Lv = 人間味のある、少し甘い読み
- 高Lv = 機械的で高精度な読み

という形で段階的に強くする。

---

## Prediction Mode

敵行動予測には、将来的に以下のようなモードを持たせる想定。

### 1. RANDOMISH

敵は完全最適ではなく、  
「それっぽく逃げるが、そこまで賢くはない」前提で読む。

特徴:

- 人間味がある
- 読みの精度は低い
- 低Lv向け

想定用途:

- Lv1〜Lv10 付近

---

### 2. MIXED

敵はある程度危険を避けるが、  
毎回最善手を選ぶわけではない前提で読む。

特徴:

- ランダム寄りと最適寄りの中間
- 中Lv向け
- 強すぎず、弱すぎない

想定用途:

- Lv11〜Lv15 付近

---

### 3. OPTIMAL

敵は **最適に逃げる / 最適に行動する** 前提で読む。

特徴:

- かなり高精度
- ハメや詰み判定に向く
- 高Lv向け

想定用途:

- Lv16〜Lv20 付近

---

## レベルごとの大まかな方針

### Lv1〜Lv10

- 敵行動予測はほぼ使わない、または RANDOMISH
- 自分の安全重視
- 低精度な敵読み

### Lv11〜Lv15

- 敵行動予測を部分的に導入
- RANDOMISH と MIXED を中心に使う
- 敵の逃げ道を少し意識する

### Lv16〜Lv20

- 敵行動予測を本格導入
- MIXED 〜 OPTIMAL を使う
- 敵の逃げ先や進路封鎖を強く意識する

---

## この設計を採用する理由

### 1. 低Lvに人間味を出せる

低Lvから最適読みを使うと、  
不自然に強いCPUになりやすい。

ランダム寄りの読みを使うことで、

- 弱め
- 少し甘い
- 人間っぽい

CPUらしさを出しやすくする。

### 2. 高Lvの強さを際立たせられる

高Lvで最適読みを解禁すると、

- 敵の逃げ道読み
- 詰みルート構築
- 高精度な爆弾設置

が可能になり、  
「高Lvだけ別格に強い」CPUを作りやすい。

### 3. 技術解禁と相性が良い

敵読み精度の向上は、

- 誘爆技
- デスサイズ系
- ボムテイル
- Exploding Punch
- Right Timing Exploding Punch

などの上級テクニックと相性が良い。

---

## 将来的にやりたいこと

敵行動予測は、将来的に以下のような用途に使う。

- 敵の逃げ先予測
- 敵の安全マス予測
- 敵の進路妨害
- 敵の詰み判定
- 敵を逃げられない位置へ追い込む爆弾設置
- 敵EscapeMapの生成
- KillMapの生成

---

## 注意

この方針は現時点では **ドラフト（仮案）** である。

実際には、

- テストプレイ
- 動画検証
- AI同士の対戦
- 対人戦での確認

を通じて、

- RANDOMISH / MIXED / OPTIMAL の具体的な中身
- 各Lvでどのモードを使うか
- どれくらいの割合で混ぜるか

を今後調整していく。

つまりこの設計は固定ではなく、  
後から自由に調整・改良してよいものとする。

---

# Enemy EscapeMap / KillMap (Draft)

将来的にAIの強さを大きく引き上げるために、  
**敵視点のマップ解析**を導入する予定。

これは、単に「自分が安全かどうか」だけではなく、

- 敵がどこに逃げるか
- 敵がどこなら助かるか
- 敵をどこに追い込めるか

を解析する仕組みである。

---

# EscapeMap

EscapeMap とは、

**敵が爆弾を避けて逃げられるマスの集合**

を表すマップである。

つまり、

「敵が生き残れる場所」

をAIが理解するための情報。

---

## EscapeMapの用途

EscapeMapを使うことで、AIは以下を判断できる。

- 敵が安全に逃げられるマス
- 敵の逃げ道の広さ
- 敵の逃げ先の候補
- 敵の安全ルート

これにより、

- 敵を追い込む
- 逃げ道を塞ぐ
- 爆弾配置を最適化する

といった判断が可能になる。

---

### EscapeMapのイメージ

例えば敵の周囲がこのような状況の場合。

###########
#.........#
#.#.#.#.#.#
#.........#
#.#.#.#.#.#
#....E....#
#.#.#.#.#.#
#.........#
#.#.#.#.#.#
#.........#
###########

- `.` = 敵が逃げられるマス
- `E` = 敵の現在位置
- `#` = 壁

このように EscapeMap は、

- 敵が生存可能なマス
- 敵の逃げ先
- 敵の安全ルート

を可視化するマップである。

AI はこの情報を使って、

- 敵の逃げ道を減らす
- 逃げ先を塞ぐ爆弾配置
- 敵を狭い場所に追い込む

といった判断を行う。

---

# KillMap

KillMap は

「敵を倒せるマス」

を表すマップである。

EscapeMap が

「敵が生き残れる場所」

を表すのに対し、

KillMap は

「敵が逃げられない領域」

を表す。

AIは

EscapeMap
↓
KillMap

を組み合わせて、

- 敵を追い込む
- 爆弾を置く位置を決める
- トラップを形成する

といった攻撃判断を行う。

---

# CPU Level Design (Planned)

将来的に AI の強さを Lv1〜Lv20 の段階で調整できるようにする計画。

CPUレベルは以下の要素で差をつける。

- 思考の賢さ
- 危険予測
- 敵行動の考慮
- 使用テクニック
- ミス率
- 先読み深さ

---

## CPU Level Table (Draft)

| Lv | 思考レベル | 主な行動 |
|----|-------------|-----------|
| 1 | 回避のみ | 危険回避のみ |
| 2 | 回避 | 単純移動 |
| 3 | 回避 + 爆弾 | 安全なときだけ爆弾 |
| 4 | 回避 + 爆弾 | 単純な置き逃げ |
| 5 | 回避強化 | 逃げ判断が少し賢い |
| 6 | 攻撃開始 | 敵が近いと爆弾 |
| 7 | テクニック | Bomb Tail |
| 8 | 攻撃強化 | 置き逃げ判断改善 |
| 9 | 誘爆技 | Throw Chain |
| 10 | 誘爆技 | Punch Chain |
| 11 | 敵視点導入 | 敵の逃げ先を少し考慮 |
| 12 | 誘爆強化 | Kick Chain |
| 13 | 敵予測 | 敵の逃げ道を意識 |
| 14 | 上級技 | Throw Death Size |
| 15 | 上級技 | Punch Death Size |
| 16 | 上級技 | Kick Death Size |
| 17 | 高難度技 | Exploding Punch |
| 18 | 高難度技 | Exploding Punch精度向上 |
| 19 | 超高難度技 | Right Timing Exploding Punch |
| 20 | 最強モード | 全テクニック + 敵予測最大 |

---

## 思考モードの段階

### Lv1〜Lv10

自分中心のAI。

- 自分の安全確保
- 単純な爆弾設置
- 単純な回避

敵の行動はほぼ考慮しない。

---

### Lv11〜Lv16

敵視点を一部導入。

- 敵の逃げ先を予測
- 進路妨害
- 誘爆技を使用

---

### Lv17〜Lv20

高度な戦術AI。

- 上級テクニック
- 敵の逃げ道封鎖
- 複合戦術
- 高精度タイミング技

---

## 調整について

このCPUレベル設計は暫定案であり、

- テクニック解禁Lv
- 敵視点導入Lv
- 思考の深さ
- ミス率

などは今後のテストや動画検証を元に調整していく。


# Trap AI Design (Draft)

Trap AI は、敵を単純に爆風へ入れるだけでなく、
敵の逃げ道を減らし、最終的に逃げられない状況へ追い込む AI を目的とする。

この AI は以下の情報を組み合わせて動作する。

Enemy EscapeMap

KillMap

Enemy Prediction

DangerMap

## Trap AI の基本思想

ボンバーマンの強い攻撃は、単純な爆弾設置ではなく、

敵の逃げ先を読む

逃げ道を減らす

狭い場所へ追い込む

最後に逃げられない配置を作る

ことで成立する。

Trap AI はこの流れを AI ロジックとして実装する。

## Trap AI の基本フロー

現在の敵位置を取得

Enemy EscapeMap を作成

敵が逃げられる範囲を計算

爆弾を置いた場合の EscapeMap を再計算

EscapeMap が大きく減る配置を高評価

自分が安全か確認

条件が良ければ Trap 用爆弾を設置

## Trap AI の目的

Trap AI の主な目的は以下。

敵の逃げ道を減らす

敵の安全地帯を狭くする

通路や角へ追い込む

次の爆弾や誘爆につながる形を作る

最終的に KillMap の成立率を上げる

## Trap AI の評価項目

爆弾配置候補は以下の観点で評価する。

### 1 EscapeMap 縮小量

敵が逃げられるマス数

安全地帯

通路幅

EscapeMap を大きく減らす配置を高評価とする。

### 2 地形圧迫

敵を以下の場所へ追い込めるかを評価する。

通路

角

十字路

壁際

### 3 自己安全性

置いた後逃げ切れるか

誘爆込みでも死なないか

自滅配置になっていないか

### 4 次の攻撃への接続

次の攻撃につながる配置は評価を上げる。

例

Kick Chain

Throw Death Size

Bomb Tail

誘爆連鎖

Trap AI の強さ段階

Trap AI は段階的に導入する。

### Trap Lv1

敵の近くに爆弾を置く

少しでも逃げ道が減れば採用

単純圧迫

### Trap Lv2

Enemy EscapeMap を参照

逃げ道が大きく減る配置を優先

通路封鎖を狙う

### Trap Lv3

Enemy Prediction を利用

敵が逃げそうな方向を先回り

Bomb Tail や Chain 技へ接続

### Trap Lv4

高度テクニックと連動

Death Size 系

Exploding Punch 系

高精度追い込み

## Trap AI の具体例
### 通路封鎖

敵が通路にいる場合
出口側に爆弾を置くことで逃げ道を減らす。

即キルでなくても Trap として有効。

### 角追い込み

敵を角方向へ寄せる配置を優先する。

角では逃げ方向が少なくなるため
次の攻撃成功率が高い。

### EscapeMap 縮小

敵の EscapeMap が広い場合
直接キルよりも EscapeMap 縮小を優先する。

### 技接続

現在キルできなくても

Kick Chain

Throw Death Size

Bomb Tail

などに接続できる配置は高評価とする。

## Trap AI と CPU Level の関係
低Lv

Trap AI ほぼ未使用

単純な爆弾配置

中Lv

簡単な通路封鎖

EscapeMap 簡易利用

高Lv

Enemy EscapeMap 使用

Enemy Prediction 使用

高度テクニック連動

## Trap AI と Enemy Prediction Policy

Trap AI は敵予測と連動する。

RANDOMISH → 甘い Trap

MIXED → 中程度 Trap

OPTIMAL → 最適逃げも潰す Trap

高Lvほど Trap 精度が上がる。

## Trap AI と Advanced Techniques

Trap AI は以下の技の成功率を上げる。

Bomb Tail

Throw Chain

Punch Chain

Kick Chain

Throw Death Size

Punch Death Size

Kick Death Size

Exploding Punch

Right Timing Exploding Punch

## 今後の実装方針

Trap AI は次の順で実装を強化する。

1 通路封鎖
2 EscapeMap 縮小評価
3 Enemy Prediction 連携
4 Chain 技連携
5 Death Size 連携
6 Exploding Punch 連携

## 注意

この設計はドラフトであり
実装・動画検証・対人テストで調整される。

特に以下を検証予定。

Trap 評価基準

EscapeMap 縮小量の閾値

直接キル以外の Trap 評価

技接続条件

CPU Level ごとの使用頻度

# Enemy Future Map

Enemy Future Map は、
敵が数フレーム後に存在している可能性が高い位置を予測するマップである。

DangerMap が「爆風の未来」を扱うのに対し、
Enemy Future Map は「敵の未来位置」を扱う。

このマップを使うことで AI は

- 敵の逃げ方向を読む
- 先回りして爆弾を置く
- Trap AI の精度を上げる

ことができる。


## Enemy Future Map の目的

Enemy Future Map の主な目的は以下である。

- 敵の未来位置予測
- Trap AI の強化
- KillMap の成功率向上
- 爆弾の先置き判断


## 基本アイデア

敵は DangerMap を避けながら移動する。

つまり敵は

- 安全マス
- EscapeMap
- 壁や通路構造

に従って動く可能性が高い。

Enemy Future Map は

「敵が行けるマス」

を BFS で展開することで作る。


## Enemy Future Map の生成

生成手順は以下。

1. 敵の現在位置を取得

2. EscapeMap を取得

3. Time-aware BFS を行う

4. 各フレームで敵が到達可能なマスを記録


結果として

EnemyFuture[x][y][t]

のような形で保存する。


## 例

敵がこの位置にいる場合

###########
#.........#
#.#.#.#.#.#
#....E....#
#.#.#.#.#.#
#.........#
###########

E = 敵の現在位置


数フレーム後には

###########
#...E.....#
#.#.#.#.#.#
#.........#
#.#.#.#.#.#
#.....E...#
###########

のような未来位置候補が得られる。


## Trap AI との関係

Trap AI は Enemy Future Map を使うことで

- 敵が来そうな場所
- 敵が逃げる方向

を予測できる。

その結果

- 通路封鎖
- 角追い込み
- EscapeMap 縮小

の精度が上がる。


## KillMap との関係

Enemy Future Map と KillMap を組み合わせると

「未来の敵位置に爆風が重なるか」

を判定できる。

これにより

- 先読みキル
- 誘爆キル
- Trap → Kill

などの判断が可能になる。


## CPU レベルとの関係

CPU レベルによって Future Map の精度を変える。

### 低レベル

敵の未来予測を行わない。

### 中レベル

数フレームのみ予測する。

### 高レベル

DangerMap を考慮した未来予測を行う。

### 最高レベル

Enemy Prediction Policy と連動する。


## 注意

Enemy Future Map はあくまで予測であり、
実際の敵行動とは一致しない可能性がある。

そのため

- RANDOMISH
- MIXED
- OPTIMAL

などの Enemy Prediction Policy と組み合わせて使用する。

# Position Evaluation Map (Draft)

Position Evaluation Map は、

「盤面上の各マスがどれくらい有利か」

を数値化するためのマップである。

DangerMap が「危険かどうか」を表し、  
EscapeMap が「逃げられる場所」を表すのに対し、

Position Evaluation Map は

「戦術的に有利な位置」

を評価するための情報である。


## Position Evaluation Map の目的

Position Evaluation Map の目的は以下。

・有利な位置を評価する  
・危険すぎる位置を避ける  
・敵を追い込みやすい位置を選ぶ  
・Trap AI の配置精度を上げる  


## 基本アイデア

ボンバーマンでは

同じ安全マスでも

・強い場所  
・弱い場所  

が存在する。

例

・通路中央  
・十字路  
・角  
・壁際  

などで戦術価値が異なる。

Position Evaluation Map は

各マスにスコアを付けることで  
AI が有利な場所を選びやすくする。


## データ構造

概念的には以下のような配列になる。

position_score[y][x]

値

高い = 有利  
低い = 不利


## 評価要素

Position Evaluation Map は  
以下の要素を組み合わせて評価する。


### 1 通路評価

通路は逃げ方向が少ないため  
危険度が高い。

そのため

・通路奥  
・袋小路  

などは低評価。


### 2 十字路評価

十字路は逃げ方向が多く  
安全度が高い。

そのため

十字路中央は高評価。


### 3 壁際評価

壁際は逃げ方向が減るため  
やや低評価。


### 4 敵との距離

敵との距離によって評価を変える。

例

敵に近すぎる → 低評価  
適度な距離 → 高評価  


### 5 EscapeMap連携

EscapeMap が広い場所は  
生存率が高い。

そのため

EscapeMap が広いマスは  
高評価。


## Trap AI との関係

Trap AI は

Enemy EscapeMap  
Enemy Future Map  
Position Evaluation Map  

を組み合わせて

・敵を追い込む  
・逃げ道を減らす  
・有利位置に誘導する  

といった戦術判断を行う。


## KillMap との関係

KillMap と組み合わせることで

・敵が不利な位置にいるか  
・次の爆弾で詰みになるか  

を判断しやすくなる。


## CPU Level との関係

CPU レベルによって  
Position Evaluation の使用度を変える。

低Lv

評価を使わない  
ランダム寄り移動

中Lv

簡単な位置評価を使用

高Lv

Position Evaluation Map を本格利用

最高Lv

Enemy Future Map と連動した評価


## 注意

Position Evaluation Map は

DangerMap  
EscapeMap  
Enemy Future Map  

などの情報と組み合わせて使用する。

単独では戦術判断を行わない。

# AI Decision System (Draft)

AI Decision System は、

DangerMap
EscapeMap
Enemy Future Map
Position Evaluation Map
KillMap

などの情報を統合し、

「次に取る行動」

を決定するロジックである。


## AI Decision System の目的

AI Decision System の役割は以下。

・回避行動の判断  
・爆弾設置判断  
・Trap 行動判断  
・待機行動判断  

複数の情報を組み合わせて  
最適な行動を選択する。


## 基本構造

AI の判断は大きく以下の流れで行う。

1 現在状態の取得

2 DangerMap で危険判定

3 EscapeMap で安全ルート確認

4 Position Evaluation Map で位置評価

5 Enemy Future Map で敵行動予測

6 KillMap で攻撃可能性確認

7 最終行動決定


## 判断優先順位

基本的な優先順位は以下。

1 生存（回避）

2 攻撃可能なら攻撃

3 Trap 形成

4 有利位置へ移動

5 待機


## 回避判断

現在位置または近い未来が危険な場合、

AI は回避を最優先する。

DangerMap を参照し、

・未来危険マス  
・移動中に危険になるマス  

を避ける。


## 攻撃判断

以下を満たす場合、

爆弾設置を検討する。

・自分が逃げ切れる  
・敵に当たる可能性がある  
・KillMap が成立する可能性


## Trap 判断

直接キルができない場合、

Trap AI を利用する。

Enemy EscapeMap を縮小できる爆弾配置を  
高評価とする。


## 位置評価

危険でも攻撃でもない場合、

Position Evaluation Map を使って  
有利な位置へ移動する。


## WAIT 判断

以下の条件では WAIT を選択する。

・現在位置が安全  
・攻撃メリットが小さい  
・Trap が成立しない


## CPU Level との関係

CPU レベルによって  
Decision System の精度を変える。

低Lv

単純な回避優先

中Lv

簡単な攻撃判断

高Lv

Trap 判断を使用

最高Lv

Enemy Future Map を使った  
高度な戦術判断


## 将来的な拡張

AI Decision System は今後、

・Advanced Techniques  
・Enemy Prediction  
・Trap AI  

などと連動し、

より高度な戦術判断を行う予定。

# AI State Machine (Draft)

AI State Machine は、

AI が現在どの行動モードにいるか

を管理する仕組みである。

AI は常に同じ判断を行うのではなく、

状況に応じて

・回避
・攻撃
・追い込み
・位置取り
・待機

などのモードを切り替える。


## State Machine の目的

State Machine を使う目的は以下。

・AI の行動を整理する  
・判断ロジックをシンプルにする  
・CPUレベルによる行動差を作る  
・戦術モードを切り替える  


## 基本ステート

AI は基本的に以下のステートを持つ。

ESCAPE  
ATTACK  
TRAP  
POSITION  
WAIT  


## ESCAPE

ESCAPE は

「回避モード」

である。

DangerMap により

・現在位置が危険  
・近い未来で危険  

と判断された場合に入る。

このモードでは

・安全マス探索  
・最短回避ルート

を優先する。


## ATTACK

ATTACK は

「攻撃モード」

である。

以下の条件を満たす場合に入る。

・爆弾設置後に逃げ切れる  
・敵に当たる可能性がある  
・KillMap が成立する可能性

このモードでは

敵ヒットを優先した爆弾設置を行う。


## TRAP

TRAP は

「追い込みモード」

である。

直接キルできない場合でも、

Enemy EscapeMap

Enemy Future Map

を使い、

・敵の逃げ道を減らす  
・通路封鎖  
・角追い込み  

などを行う。


## POSITION

POSITION は

「有利位置取りモード」

である。

Position Evaluation Map を使い、

・安全  
・有利  
・戦術価値が高い  

マスへ移動する。


## WAIT

WAIT は

「待機モード」

である。

以下の場合に入る。

・現在位置が安全  
・攻撃メリットが小さい  
・Trap が成立しない

無理に動かず  
状況を観察する。


## State 遷移

基本的なステート遷移は以下。

ESCAPE
↓
POSITION
↓
ATTACK
↓
TRAP
↓
WAIT


ただし、

DangerMap によって危険と判定された場合は  
どのステートからでも ESCAPE に戻る。


## CPU Level との関係

CPU レベルにより  
State Machine の使用範囲を変える。

低Lv

ESCAPE  
WAIT

のみ使用


中Lv

ESCAPE  
ATTACK  
WAIT


高Lv

ESCAPE  
ATTACK  
TRAP  
POSITION


最高Lv

すべてのステートを使用し、

Enemy Prediction  
Trap AI  
Advanced Techniques

と連動する。


## 将来的な拡張

State Machine は将来的に

・Advanced Techniques  
・Enemy Prediction  
・Trap AI  

などと統合され、

より高度な戦術AIとして発展させる予定。

# AI Data Flow (Draft)

AI Data Flow は、

ゲーム状態から AI の最終行動が決定されるまでの

「情報の流れ」

を整理したものである。

この AI は、

・ゲーム状態取得  
・未来予測  
・マップ解析  
・戦術評価  
・行動決定  

という段階を経て行動を決定する。


## 全体フロー

AI の基本的な処理フローは以下。

Game State
↓
Bomb Simulator
↓
DangerMap
↓
EscapeMap
↓
Enemy Future Map
↓
Position Evaluation Map
↓
KillMap
↓
AI Decision System
↓
AI State Machine
↓
Controller Output


## Game State

現在のゲーム情報を取得する。

主な情報

・自分の位置  
・敵の位置  
・爆弾の位置  
・爆弾タイマー  
・盤面構造  


## Bomb Simulator

現在の爆弾状態をもとに、

未来の爆発をシミュレーションする。

計算内容

・爆発フレーム  
・誘爆  
・爆風範囲  


## DangerMap

DangerMap は

未来フレームごとの危険マス

を記録する。

danger[frame][y][x]


## EscapeMap

EscapeMap は

敵が安全に逃げられるマス

を表す。


## Enemy Future Map

Enemy Future Map は

敵の未来位置の候補

を予測するマップ。


## Position Evaluation Map

Position Evaluation Map は

盤面上の各マスの戦術価値

を数値化する。


## KillMap

KillMap は

敵を倒せる可能性があるマス

を表す。


## AI Decision System

AI Decision System は

各種マップ情報を統合し、

次の行動候補を評価する。


## AI State Machine

AI State Machine は

現在の戦術モードを管理する。

例

ESCAPE  
ATTACK  
TRAP  
POSITION  
WAIT  


## Controller Output

最終的に決定した行動を

コントローラー入力

へ変換する。


## scp_driver

この AI は

SCP Virtual Bus Driver を使用し、

仮想コントローラー入力として

ゲームへ行動を送信する。


## 将来的な拡張

AI Data Flow は今後

・Enemy Prediction  
・Trap AI  
・Advanced Techniques  

などと統合され、

より高度な戦術 AI を構成する予定。

# AI Implementation Roadmap (Draft)

この章では、GGAI を今後どの順番で実装・強化していくかを整理する。

README に多くの設計章が追加されているが、  
実装は一度にすべて行うのではなく、段階的に進める。

---

## Phase 1: 基本回避AI

目的

- DangerMap による未来危険判定
- Time-aware BFS による安全経路探索
- Escape Route Reconstruction による最初の1手抽出

状態

- 実装済み

主な機能

- 危険マスを避ける
- 安全マスへ移動する
- 「危険なら逃げる」という基本判断

---

## Phase 2: 爆弾設置AI

目的

- 爆弾を置いた後に自分が逃げ切れるか確認する
- 逃げ切れる場合のみ爆弾を置く

状態

- 実装中 / 次の主目標

主な機能

- 仮想爆弾設置
- 新しい DangerMap の再生成
- PathFinder で逃走可能判定
- PLACE_BOMB / WAIT の判断

---

## Phase 3: 攻撃判断AI

目的

- ただ置くだけでなく
  敵に当たる可能性がある爆弾を優先する

主な機能

- KillMap の利用
- Enemy Future Map の利用
- 敵ヒット確率を考慮した設置

---

## Phase 4: Trap AI

目的

- 敵の逃げ道を減らす
- 通路封鎖や角追い込みを行う

主な機能

- Enemy EscapeMap
- Trap AI
- Position Evaluation Map
- Enemy Prediction

---

## Phase 5: Enemy Prediction 強化

目的

- 敵の未来位置予測の精度を上げる
- CPU レベルごとの差を作る

主な機能

- Enemy Prediction Architecture
- Enemy Prediction Policy
- RANDOMISH / MIXED / OPTIMAL
- CPU Level 連動

---

## Phase 6: 高度戦術AI

目的

- AI Decision System
- AI State Machine
- AI Data Flow
- PathFinder Architecture

などを統合し、より高度な判断を行う

主な機能

- ESCAPE
- ATTACK
- TRAP
- POSITION
- WAIT

の戦術切り替え

---

## Phase 7: Advanced Techniques

目的

- 上級テクニックを AI が使えるようにする

対象技

- Bomb Tail
- Throw Chain
- Punch Chain
- Kick Chain
- Throw Death Size
- Punch Death Size
- Kick Death Size
- Exploding Punch
- Right Timing Exploding Punch

---

## Phase 8: CPU Level 完成

目的

- Lv1〜Lv20 の強さ差を完成させる
- 敵予測精度
- 使用技
- ランダム性
- Trap 精度

などを段階調整する

---

## 最終目標

最終的には以下を備えた AI を目指す。

- 高精度な未来危険予測
- 安全な回避
- 爆弾設置後の生存確認
- 敵未来位置予測
- Trap AI
- 上級テクニック使用
- CPU レベル別難易度制御

---

## 現在位置

現在の主な実装段階は

Phase 2: 爆弾設置AI

である。

つまり次にやるべき最重要タスクは

「置いた後に逃げられる場合のみ PLACE_BOMB する AI」

の実装である。

# PathFinder Architecture (Draft)

PathFinder は、

AI が安全に移動するための経路を探索するモジュールである。

この AI では単純な最短経路探索ではなく、

「時間を考慮した BFS（Time-aware BFS）」

を使用する。


## PathFinder の目的

PathFinder の目的は以下。

- 安全マスへの経路探索
- 爆弾回避
- 爆弾設置後の逃走確認
- 敵追跡時の移動可能範囲確認


## Time-aware BFS

通常の BFS は

(x, y)

のみを扱う。

しかしこの AI では

(x, y, frame)

を扱う。

つまり

- どこにいるか
- いつそこにいるか

を同時に探索する。


## 探索状態

探索ノードは以下。

(x, y, frame)

意味

x = 横座標  
y = 縦座標  
frame = そのマスにいる時間


## 使用する情報

PathFinder は以下を使用する。

- Board
- DangerMap
- 現在位置
- 移動仕様


## DangerMap との関係

DangerMap は

danger[frame][y][x]

で危険を管理する。

PathFinder は

そのフレームで安全か

を確認しながら探索する。


## 移動仕様

SBR2 の移動仕様

center → next = 6F  
normal move = 11F

この仕様を考慮して

到達フレームを計算する。


## 探索行動

PathFinder が扱う行動

- WAIT
- UP
- DOWN
- LEFT
- RIGHT


## 基本アルゴリズム

1. 現在位置をキューに入れる

2. BFS を開始する

3. 隣接マスを展開する

4. 到達フレームを計算する

5. DangerMap を確認する

6. 安全なら探索を続行する

7. 条件を満たすマスが見つかったら終了


## Escape Route Reconstruction

BFS の結果から

最初の行動

を復元する。


## 爆弾設置 AI との関係

爆弾を置く前に

1. 仮想爆弾を設置
2. 新しい DangerMap を生成
3. PathFinder で逃げられるか確認

を行う。


## Trap AI との関係

Trap AI では

- 自分が Trap 後に逃げられるか
- 有利位置へ移動できるか

を確認するため

PathFinder を使用する。


## CPU Level との関係

低Lv

単純な回避のみ


中Lv

爆弾設置後の逃走確認


高Lv

Trap AI  
Enemy Prediction  
と連動


## 将来的な改善

- center 6F 移動対応
- normal move 11F 正確化
- WAIT 評価改善
- 敵予測との統合


## 注意

PathFinder の精度は

DangerMap  
移動フレーム仕様

に強く依存する。

仕様がズレると

回避 AI  
設置 AI  
Trap AI

すべての精度が崩れる。

# Advanced Techniques Architecture (Draft)

Advanced Techniques は、

通常の爆弾設置 AI を超えた

「高度テクニック」

を AI が使用できるようにする設計である。

これらは単独で使うのではなく、

Trap AI  
Enemy Prediction  
Position Evaluation

などと組み合わせて使用する。


## Advanced Techniques の目的

Advanced Techniques の主な目的は以下。

- 高度な爆弾テクニックの使用
- 追い込み状況でのキル確率向上
- Trap AI の成功率向上
- 人間上級者に近い戦術の再現


## 基本思想

SBR2 では

単純な爆弾設置だけでは
上級プレイヤーを倒すことは難しい。

そのため AI は

爆弾操作系テクニック

を使用する必要がある。


## 主なテクニック

AI が将来的に扱うテクニックは以下。


### Bomb Tail

爆弾を連続設置して

通路を封鎖する技。

敵の EscapeMap を急激に縮小できる。


### Kick Chain

Kick を利用して

爆弾を連鎖的に動かす技。

通路封鎖や追い込みに使う。


### Throw Chain

Throw を利用して

遠距離爆弾連鎖を作る技。


### Punch Chain

Punch によって

爆弾の連鎖配置を作る技。


### Death Size 系

Throw Death Size  
Kick Death Size  
Punch Death Size

などの

高難度キル技。


### Exploding Punch

爆弾誘爆タイミングを利用して

瞬時爆発を作るテクニック。


### Right Timing Exploding Punch

Exploding Punch の

タイミング最適化版。


## Advanced Techniques の実行条件

これらのテクニックは

以下の条件が揃った場合のみ実行する。

- EscapeMap が有利
- KillMap が成立する可能性
- 自分の逃走経路が存在
- Enemy Future Map が一致


## AI Decision System との関係

Advanced Techniques は

AI Decision System の

「高優先度行動候補」

として扱われる。


## State Machine との関係

Advanced Techniques は

主に以下の状態で使用される。

ATTACK  
TRAP


## CPU Level との関係

低Lv

使用しない


中Lv

簡単な Chain 技のみ


高Lv

高度テクニックを使用


最高Lv

Enemy Prediction と統合


## 注意

Advanced Techniques は

計算量が大きくなる可能性がある。

そのため

CPU Level  
AI Difficulty

に応じて

使用頻度を制御する必要がある。

# Enemy Prediction Architecture (Draft)

Enemy Prediction は、

敵プレイヤーの未来行動を予測するための AI モジュールである。

この AI では

- EscapeMap
- PathFinder
- DangerMap

などの情報を利用して、

敵が将来どのマスへ移動する可能性があるか

を推定する。


## Enemy Prediction の目的

Enemy Prediction の目的は以下。

- 敵の逃走方向を予測する
- Trap AI の成功率を上げる
- 爆弾配置のキル確率を上げる
- Position Evaluation の精度向上


## 基本思想

ボンバーマンでは

敵がどこへ逃げるか

を予測できると

Trap  
通路封鎖  
Chain 技

などの成功率が大きく上がる。

そのため AI は

敵の未来位置を

確率的に予測する必要がある。


## Enemy Future Map との関係

Enemy Prediction は

Enemy Future Map

を生成するためのモジュールである。

Enemy Future Map は

enemy_future[frame][y][x]

として管理される。


## 基本アルゴリズム

Enemy Prediction は

以下の流れで計算される。

1. 敵の現在位置を取得

2. EscapeMap を参照

3. PathFinder を使用して
   敵が移動可能なマスを探索

4. 各フレームの
   未来到達可能マスを計算

5. Enemy Future Map を生成


## 予測対象フレーム

Enemy Prediction は

DangerMap と同じく

最大 512 フレーム

まで予測する。


## 予測対象行動

敵が取り得る行動は以下。

- WAIT
- UP
- DOWN
- LEFT
- RIGHT

将来的には

Kick  
Throw  
Punch

などの行動も

予測対象になる可能性がある。


## DangerMap との関係

Enemy Prediction は

DangerMap を参照して

敵が危険マスを避ける可能性

を考慮する。

つまり

敵は基本的に

危険マスへ自ら入らない

と仮定する。


## EscapeMap との関係

EscapeMap は

敵が安全に移動できる範囲

を示す。

Enemy Prediction は

この範囲を中心に

未来位置を生成する。


## Position Evaluation との関係

Enemy Future Map は

Position Evaluation Map の

重要な入力情報になる。

これにより AI は

敵が来る可能性の高い位置

を高く評価できる。


## Trap AI との関係

Trap AI は

Enemy Future Map を利用して

敵が逃げそうな方向

を先回りして封鎖する。


## KillMap との関係

KillMap は

Enemy Future Map と

DangerMap を組み合わせて

生成される。

つまり

敵未来位置  
×  
爆風

の交差点を

キル候補として扱う。


## CPU Level との関係

Enemy Prediction の精度は

CPU Level によって変わる。

低Lv

単純な近距離予測のみ


中Lv

EscapeMap を利用


高Lv

PathFinder を利用した
広範囲予測


最高Lv

Trap AI  
Advanced Techniques

と統合


## 計算コスト

Enemy Prediction は

計算量が大きくなる可能性がある。

そのため

- 予測フレーム数
- 探索深度

を CPU Level に応じて制御する。


## 将来的な拡張

Enemy Prediction は今後

- プレイヤー行動パターン分析
- プレイヤー癖学習
- 技使用確率予測

などへ拡張される可能性がある。


## 注意

Enemy Prediction は

あくまで予測であり

実際のプレイヤー行動と

完全に一致するとは限らない。

そのため AI は

複数の未来可能性

を同時に扱う必要がある。

# Enemy Prediction Policy (Draft)

Enemy Prediction Policy は、

Enemy Prediction Architecture によって生成された

敵未来予測情報

を、どのような前提で使うかを定義する方針である。

同じ Enemy Future Map を使う場合でも、

- 敵をどれくらい賢いと仮定するか
- どれくらいランダムに動くと仮定するか
- どの程度の精度で先読みするか

によって、AI の強さや人間らしさは大きく変わる。


## Policy の目的

Enemy Prediction Policy の目的は以下。

- CPU レベルごとの差を作る
- 敵予測の強さを制御する
- 人間らしい甘さを残す
- 高レベル CPU に最適予測を持たせる


## 基本思想

低レベル CPU では

敵の行動を完全には読めない方が自然である。

一方で高レベル CPU では

敵の逃げ先をかなり正確に読むことで

Trap AI  
KillMap  
Advanced Techniques

の成功率を高めることができる。

そのため Enemy Prediction には

複数の Policy を持たせる。


## 主な Prediction Policy

### RANDOMISH

RANDOMISH は

敵がややランダムに動く

と仮定するモードである。

特徴

- 人間らしい甘さがある
- 予測精度は低い
- 敵の移動候補を広めに取る
- Trap 精度は低め

主な用途

- 低レベル CPU
- 初級者向け


### MIXED

MIXED は

敵がある程度合理的に動くが、

完全最適ではない

と仮定するモードである。

特徴

- ランダムと最適の中間
- 中程度の精度
- 危険回避は行う
- 完全な読み切りはしない

主な用途

- 中レベル CPU
- 中級者向け


### OPTIMAL

OPTIMAL は

敵が最適行動を取る

と仮定するモードである。

特徴

- 最も高精度
- 危険回避を最大限考慮
- EscapeMap を強く意識
- Trap AI や KillMap の精度が高い

主な用途

- 高レベル CPU
- 上級者向け


## CPU Level との関係

Enemy Prediction Policy は

CPU Level に応じて切り替える。

### 低Lv

主に RANDOMISH

- 敵を甘く読む
- ランダム寄り
- 人間らしい弱さを残す


### 中Lv

主に MIXED

- 一部だけ最適予測
- 一部ランダム
- 強すぎず弱すぎない


### 高Lv

主に OPTIMAL

- 敵の最適逃げを仮定
- Trap 精度向上
- KillMap 成功率向上


## Trap AI との関係

Trap AI は Enemy Prediction Policy によって強さが変わる。

RANDOMISH

- 甘い Trap
- 封鎖精度が低い

MIXED

- それなりに追い込む
- 通路封鎖が安定する

OPTIMAL

- 最適逃げすら潰す
- 高精度 Trap が可能


## KillMap との関係

KillMap も Policy に影響される。

RANDOMISH

- キル候補が広くなる
- 精度は低い

MIXED

- キル候補が中程度に絞られる

OPTIMAL

- 最も鋭い KillMap
- 先読みキル精度が高い


## Position Evaluation Map との関係

Position Evaluation Map は

Enemy Prediction Policy によって

敵の未来位置の重み付け

を変えることができる。

例えば

RANDOMISH では広く分散した評価

OPTIMAL では集中した評価

になる。


## 予測の揺らぎ

Enemy Prediction Policy では

完全固定の予測だけでなく、

揺らぎ

を加えることも考えられる。

例

- 一定確率で予測を外す
- 候補マスを広めに取る
- 近距離だけ精度を上げる

これにより CPU の個性や人間らしさを出せる。


## 将来的な拡張

将来的には

- プレイヤーごとの癖学習
- 攻撃型 / 回避型の分類
- Technique 使用傾向の予測

なども Enemy Prediction Policy に統合する可能性がある。


## 注意

Enemy Prediction Policy は

Enemy Prediction Architecture が生成した情報を

どう解釈するか

を定義するものである。

つまり

Architecture = 未来候補を作る仕組み  
Policy = その未来候補の使い方

である。

この2つは役割が異なるが、  
密接に連動する。