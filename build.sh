#!/bin/bash
# HelperBoard A133 RGB Display Program - Build Script
# 编译脚本

echo "========================================="
echo "  RGB Display Program - Build Script"
echo "  HelperBoard A133"
echo "========================================="
echo ""

# 项目目录
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "Project directory: $PROJECT_DIR"
echo ""

# 清理旧的构建文件
echo "Cleaning old build files..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 进入构建目录
cd "$BUILD_DIR"

# 生成 Makefile
echo ""
echo "Generating Makefile..."
qmake "$PROJECT_DIR/A133_BASE.pro"

if [ $? -ne 0 ]; then
    echo "❌ qmake failed!"
    exit 1
fi

# 编译
echo ""
echo "Compiling..."
make -j2

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed!"
    exit 1
fi

# 检查可执行文件
if [ ! -f "A133_BASE" ]; then
    echo "❌ Executable not found!"
    exit 1
fi

# 复制到主目录
echo ""
echo "Copying executable to home directory..."
cp "A133_BASE" "$HOME/"
chmod +x "$HOME/A133_BASE"

echo ""
echo "========================================="
echo "  ✅ Build Success!"
echo "========================================="
echo ""
echo "Executable location: $HOME/A133_BASE"
echo ""
echo "To run the program:"
echo "  cd $HOME"
echo "  ./A133_BASE"
echo ""
echo "Or use the run script:"
echo "  cd $PROJECT_DIR"
echo "  ./run.sh"
echo ""
