#!/bin/bash
# HelperBoard A133 RGB Display Program - Build and Run Script
# 编译并运行脚本

echo "========================================="
echo "  RGB Display Program"
echo "  Build and Run Script"
echo "  HelperBoard A133"
echo "========================================="
echo ""

# 项目目录
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 停止 qmlui 服务
echo "Stopping qmlui service..."
systemctl stop qmlui 2>/dev/null || true

# 先编译
echo "Step 1: Building..."
echo ""

"$PROJECT_DIR/build.sh"

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Build failed!"
    exit 1
fi

# 再运行
echo ""
echo "Step 2: Running..."
echo ""

"$PROJECT_DIR/run.sh"
