# VT-2B Black

**EMU AUDIO** - Console Bus Glue Processor

![VT-2B Black](./preview.png)

---

## 概要

VT-2B Blackは、コンソールサミングやバス回路を意識した**密度増加型サチュレーションプロセッサ**です。

派手なサチュレーションではなく、**音をまとめる・整える**ことに特化した設計。バス/マスターに常時挿しておけるプロ仕様のプラグインです。

### キャラクター
- **Glue** - 音を一つにまとめる
- **Tight** - タイトで引き締まった
- **Controlled** - 制御された、暴れない
- **Modern** - モダンで洗練された

---

## 機能

### Drive (0.0 - 10.0)
密度増加の量をコントロール。上げるほど音の粒立ちが揃い、まとまり感が増します。

| 範囲 | 効果 |
|------|------|
| 0-2 | ほぼ透明、わずかな密度感 |
| 2-5 | 音がまとまってくる、Glue感 |
| 5-8 | 明確な密度増加 |
| 8-10 | 最大密度 |

### Mix (0% - 100%)
Dry/Wetミックス。パラレル処理対応。

---

## 技術仕様

### DSP処理
- **密度増加型サチュレーション** - テープ系の柔らかい飽和特性
- **低次倍音生成** - 2次/3次倍音を微量付加
- **トランジェント整形** - ピークの暴れを抑制
- **位相安定化** - オールパスフィルタによるステレオ像維持
- **自動ゲイン補償** - Drive変更時の音量変化を相殺

### フォーマット
- VST3
- Audio Unit (AU)
- Standalone

### 動作環境
- macOS 10.13以降
- Windows 10以降（対応予定）
- サンプルレート: 44.1kHz ~ 192kHz

---

## ビルド

### 必要要件
- CMake 3.22以降
- C++17対応コンパイラ
- Xcode (macOS) または Visual Studio (Windows)

### macOS

```bash
# ビルドスクリプト実行
chmod +x build.sh
./build.sh

# または手動でCMake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### プラグインのインストール

ビルド後、生成されたプラグインを以下にコピー：

- **VST3**: `~/Library/Audio/Plug-Ins/VST3/`
- **AU**: `~/Library/Audio/Plug-Ins/Components/`

---

## 推奨使用シナリオ

| シナリオ | Drive | Mix | 効果 |
|----------|-------|-----|------|
| ドラムバス | 4.0 | 70% | グルー感、まとまり |
| ステムミックス | 2.5 | 50% | 自然な密度 |
| マスター（控えめ） | 1.5 | 40% | 最終仕上げ |
| ボーカルバス | 3.0 | 60% | コーラス統一感 |

---

## ライセンス

Copyright © 2026 EMU AUDIO. All rights reserved.

---

## 開発

```
vt2b-black-ui/
├── CMakeLists.txt        # ビルド設定
├── build.sh              # macOSビルドスクリプト
├── README.md             # このファイル
├── DSP_DESIGN.md         # DSP設計書
├── src/
│   ├── PluginProcessor.h     # DSPヘッダ
│   ├── PluginProcessor.cpp   # DSP実装
│   ├── PluginEditor.h        # UIヘッダ
│   └── PluginEditor.cpp      # UI実装
├── resources/            # 画像リソース（オプション）
├── index.html            # UIプレビュー
├── style.css
└── script.js
```
