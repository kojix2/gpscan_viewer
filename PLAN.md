
# gpscan_viewer: GrandPerspective データ・ビューワ移植計画

## 1. 目的 (Goals)

- Linux 上で **GrandPerspective が生成したスキャン結果ファイル**を読み込み、ツリーマップ（矩形分割）として可視化できる GUI ビューワを作る。
- **スキャン機能は実装しない**（ディレクトリ走査・容量計算・フィルタ付き再スキャン等は対象外）。
- 最小のユースケースを短期間で成立させる: 「ファイルを開く → 描画 → クリックで情報表示」。

## 2. 非目的 (Non-Goals)

- macOS アプリ（AppKit/XIB）を Linux にそのまま移植しない。
- QuickLook、ゴミ箱移動、Finder 連携、macOS 固有のメニュー構造、設定パネル群は対象外。
- GrandPerspective の UI/UX を完全再現しない（機能は段階的に追加）。
- 生成ファイル形式を新規定義しない（既存形式に追従）。

## 3. 目標成果物 (Deliverables)

- `gpscan_viewer/` 配下に独立した C++/Qt アプリケーション（CMake ビルド）。
- 入力: GrandPerspective が出力する「Load Scan Data」相当のファイル（XML もしくは raw）。
- 出力: GUI にツリーマップ表示、基本情報（パス/サイズ）表示、画像書き出し（任意/後回し可）。
- ドキュメント: ビルド手順、対応フォーマット、既知の制限。

## 4. 技術選定

- 言語: C++17
- GUI: Qt6 Widgets
- ビルド: CMake
- XML パース: `tinyxml2` を第一候補（依存を減らしたい場合は Qt の `QXmlStreamReader` に変更可能）

理由: Qt は `QPainter`/`QImage`/イベント処理が揃っており、ズーム・パン・選択などの GUI 機能を短いコードで実装できる。

## 5. 参照元コード（仕様の出典）

### 5.1 入出力

- `src/io/TreeReader.m`（既存データの読み込み）
- `src/io/XmlTreeWriter.m`（XML フォーマットの書き出し仕様）
- `src/io/RawTreeWriter.m`（raw フォーマットの書き出し仕様）

### 5.2 モデル（ツリー構造）

- `src/tree/Item.*`, `FileItem.*`, `DirectoryItem.*`
- `src/tree/TreeContext.*`, `ScanTreeRoot.*`

### 5.3 レイアウト／描画（アルゴリズムの参考）

- `src/tree/TreeLayoutBuilder.*`（矩形分割ロジック）
- `src/view/TreeDrawer*.*`, `src/view/DirectoryView.m`（描画とインタラクション）

※本プロジェクトは描画/UI を Qt で再実装し、上記は「仕様確認・挙動確認」の参照とする。

## 6. アーキテクチャ（最小構成）

### 6.1 データフロー

1) ファイル選択（Open）
2) `TreeReader` がファイルを解析し `TreeModel` を構築
3) `TreeLayout` が表示領域へ矩形配置（各ノードへ `Rect` を割当）
4) `CanvasWidget` が `QPainter` で描画
5) 入力（クリック/ホバー/ズーム）に応じてハイライト・情報表示

### 6.2 モジュール責務

- `TreeModel`:
	- ノード構造（ディレクトリ/ファイル）
	- サイズ（logical/physical の区別がある場合は logical を優先）
	- パス（表示用文字列）
	- 子配列、親参照（任意）

- `TreeReader`:
	- XML/raw を読み込み `TreeModel` を構築
	- 例外/エラーを `Result` として返却（UI で表示）

- `TreeLayout`:
	- ツリーマップ配置（まずは「見える」ことが目的）
	- 初期は “squarify” 系に寄せるか、既存の `TreeLayoutBuilder` を部分移植
	- 拡張で表示深度、最小矩形閾値、余白などを調整

- `CanvasWidget`:
	- `paintEvent()` で矩形描画
	- クリックでノード選択、ホバーでツールチップ
	- ズーム・パン（第2フェーズ以降でも可）

- `ViewerWindow`:
	- メニュー（Open / Reload / Quit）
	- ステータスバー等に選択情報表示

## 7. ディレクトリ設計（作るもの）

```
gpscan_viewer/
	CMakeLists.txt
	README.md
	PLAN.md
	src/
		main.cpp
		ViewerWindow.h/.cpp
		CanvasWidget.h/.cpp
		TreeModel.h/.cpp
		TreeReader.h/.cpp
		TreeLayout.h/.cpp
	resources/
		(サンプルのスキャン結果ファイルを置く)
```

## 8. マイルストーン（段階的）

### M0: 足場（低）

- CMake で Qt6 Widgets アプリがビルド・起動できる
- 空のキャンバスが表示できる

### M1: ファイル読み込み（中）

- `Open…` で XML（優先）を読み込める
- パース失敗時にユーザー向けエラーを表示

### M2: 静的ツリーマップ表示（中〜高）

- ルート配下の矩形が「サイズ比」に応じて分割されて描画される
- クリックで矩形選択し、パス/サイズを表示

### M3: 使い勝手（中）

- ホバーでツールチップ
- ズーム（矩形ダブルクリックでフォーカス、戻るボタン等）

### M4: 追加（任意）

- 画像エクスポート（PNG）
- 色付けルール（拡張子/タイプ/ランダム/固定）
- raw 形式の対応

## 9. 受け入れ条件 (Acceptance Criteria)

- Linux 上で `Open` → サンプルファイルの可視化ができる。
- 表示された矩形の面積比が、ファイルサイズ比と概ね一致する（完全一致は不要だが破綻しない）。
- ノードをクリックすると、少なくとも「名前（またはパス）」「サイズ」が UI に出る。
- 異常系（ファイル不正/形式不一致）で落ちずにエラー表示する。

## 10. 主要リスクと対策

1) 入力フォーマット不明確
	 - 対策: XML 出力を最優先で対応。raw は後回し。
	 - 必要: 実際の生成ファイル（少なくとも 1 つ）を `resources/` に置く。

2) レイアウト差
	 - 対策: まずは “見える” レイアウトで良い（squarify など）。
	 - 次段階で `TreeLayoutBuilder` の挙動へ近づける。

3) 大規模データでの性能
	 - 対策: 初期は単純描画（同一深度まで）＋最小矩形閾値でカット。
	 - 将来: タイル分割、インクリメンタル描画、キャッシュ。

## 11. 次に必要な入力（ユーザー提供）

- GrandPerspective が生成したスキャン結果ファイル（XML が最優先、raw があれば追加）
	- できれば「小」「中」2種類（ノード数が違う）
	- `gpscan_viewer/resources/` に配置する

---

更新履歴:
- 2026-01-19 初版

