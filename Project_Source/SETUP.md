# VT-2B Black 開発環境セットアップガイド

## 必要なツール

### 1. Homebrew（パッケージマネージャー）
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 2. CMake（ビルドツール）
```bash
brew install cmake
```

### 3. JUCE フレームワーク
https://juce.com/get-juce/download からダウンロード

---

## ビルド方法

### オプションA: CMakeを使用（推奨）

```bash
cd /Users/manato/.gemini/antigravity/scratch/vt2b-black-ui
./build.sh
```

### オプションB: Projucerを使用

1. JUCEをダウンロード・展開
2. `Projucer.app` を起動
3. `VT-2B Black.jucer` を開く
4. 「Save and Open in IDE」をクリック
5. Xcodeでビルド (⌘B)

### オプションC: 手動でCMakeビルド

```bash
cd /Users/manato/.gemini/antigravity/scratch/vt2b-black-ui
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

---

## ビルド成果物の場所

ビルド成功後、以下にプラグインが生成されます:

```
build/VT2B_Black_artefacts/Release/
├── VST3/VT-2B Black.vst3
├── AU/VT-2B Black.component
└── Standalone/VT-2B Black.app
```

---

## プラグインのインストール

### VST3
```bash
cp -r "build/VT2B_Black_artefacts/Release/VST3/VT-2B Black.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

### Audio Unit
```bash
cp -r "build/VT2B_Black_artefacts/Release/AU/VT-2B Black.component" ~/Library/Audio/Plug-Ins/Components/
```

### キャッシュクリア（AUの場合）
```bash
killall -9 AudioComponentRegistrar
```

---

## トラブルシューティング

### CMakeが見つからない
```bash
export PATH="/opt/homebrew/bin:$PATH"
```

### JUCEが見つからない
CMakeLists.txtの`FetchContent`が自動でJUCEをダウンロードするので、初回は数分かかります。

### Xcodeがない
App StoreからXcodeをインストールしてください。
