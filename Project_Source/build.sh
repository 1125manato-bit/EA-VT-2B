#!/bin/bash
# ==============================================================================
# VT-2B Black - EMU AUDIO
# ビルドスクリプト (macOS)
# ==============================================================================

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "========================================"
echo "  VT-2B Black - Build Script"
echo "  EMU AUDIO"
echo "========================================"
echo ""

# ビルドタイプ
BUILD_TYPE=${1:-Release}
echo "Build Type: $BUILD_TYPE"
echo ""

# ビルドディレクトリ作成
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake設定
echo "[1/3] Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.13"

echo ""

# ビルド実行
echo "[2/3] Building..."
cmake --build . --config "$BUILD_TYPE" -j$(sysctl -n hw.ncpu)

echo ""

# 結果表示
echo "[3/3] Build Complete!"
echo ""
echo "Built plugins:"
find "$BUILD_DIR" -name "*.vst3" -o -name "*.component" -o -name "*.app" 2>/dev/null | head -20

echo ""
echo "========================================"
echo "  Build finished successfully!"
echo "========================================"
