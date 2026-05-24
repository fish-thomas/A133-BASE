#!/bin/bash
# HelperBoard RGB Display Program - Smart Run Script
# 自动识别运行环境并执行

echo "========================================="
echo "  RGB Display Program - Smart Run Script"
echo "========================================="
echo ""

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
HOME_EXECUTABLE="$HOME/A133_BASE"
BUILD_EXECUTABLE="$BUILD_DIR/A133_BASE"
EXECUTABLE=""

echo "Looking for executable..."
if [ -f "$BUILD_EXECUTABLE" ]; then
    echo "Found in build directory!"
    EXECUTABLE="$BUILD_EXECUTABLE"
elif [ -f "$HOME_EXECUTABLE" ]; then
    echo "Found in home directory!"
    EXECUTABLE="$HOME_EXECUTABLE"
else
    echo "❌ Executable not found!"
    echo ""
    echo "Please build first using:"
    echo "  $PROJECT_DIR/build.sh"
    exit 1
fi

echo ""
echo "========================================="
echo "  Environment Detection"
echo "========================================="

ARCH=$(uname -m)
IS_EMBEDDED=0
IS_ARM=0

echo "Architecture: $ARCH"

if [[ "$ARCH" == "aarch64" || "$ARCH" == "armv7l" || "$ARCH" == "armv6l" ]]; then
    IS_ARM=1
    IS_EMBEDDED=1
    echo "Detected: ARM Architecture (Embedded Device)"
elif [[ "$ARCH" == "x86_64" || "$ARCH" == "i686" ]]; then
    echo "Detected: x86 Architecture (PC)"
fi

if [ -z "$DISPLAY" ]; then
    IS_EMBEDDED=1
    echo "Detected: No DISPLAY (Embedded Device)"
else
    echo "Detected: DISPLAY=$DISPLAY (Desktop Linux)"
fi

if command -v Xorg &> /dev/null || command -v weston &> /dev/null; then
    if [ -n "$DISPLAY" ]; then
        IS_EMBEDDED=0
        echo "Detected: X11/Wayland Available (Desktop Linux)"
    fi
fi

QMLUI_RUNNING=0
if systemctl is-active --quiet qmlui 2>/dev/null; then
    QMLUI_RUNNING=1
    IS_EMBEDDED=1
    echo "Detected: qmlui service running (Embedded Device)"
elif pgrep -x qmlui > /dev/null 2>&1; then
    QMLUI_RUNNING=1
    IS_EMBEDDED=1
    echo "Detected: qmlui process running"
fi

if [ $QMLUI_RUNNING -eq 1 ]; then
    echo "Stopping qmlui service..."
    
    # Try with sudo first
    sudo systemctl stop qmlui 2>/dev/null || true
    sudo pkill -9 qmlui 2>/dev/null || true
    sudo killall -9 qmlui 2>/dev/null || true
    
    # Also try to kill any Qt/QML related processes
    sudo pkill -9 -f "qmlscene" 2>/dev/null || true
    sudo pkill -9 -f "qt-quick" 2>/dev/null || true
    
    sleep 1
    
    # Check if still running
    if pgrep -x qmlui > /dev/null 2>&1; then
        echo "Warning: qmlui may still be running"
    else
        echo "qmlui service stopped successfully"
    fi
fi

echo ""
echo "========================================="
echo "  Starting Application"
echo "========================================="

unset QTDIR
unset LD_LIBRARY_PATH
unset QT_QPA_PLATFORM_PLUGIN_PATH
unset QML2_IMPORT_PATH
unset QT_QPA_FONTDIR
unset QT_QPA_EGLFS_INTEGRATION
unset QT_QPA_FB_TSLIB
unset TSLIB_CONSOLEDEVICE
unset TSLIB_CONFFILE
unset TSLIB_CALIBFILE
unset TSLIB_FBDEVICE
unset TSLIB_PLUGINDIR
unset TSLIB_TSDEVICE
unset QT_QPA_GENERIC_PLUGINS

if [ $IS_EMBEDDED -eq 1 ]; then
    echo "Running in: EMBEDDED MODE (LinuxFB)"
    echo ""

    export QT_QPA_PLATFORM=linuxfb
    export QT_QPA_EGLFS_WIDTH=800
    export QT_QPA_EGLFS_HEIGHT=480
    export QT_QPA_EGLFS_DEPTH=32
else
    echo "Running in: DESKTOP MODE (XCB)"
    echo ""

    export QT_QPA_PLATFORM=xcb

    if [ -n "$DISPLAY" ]; then
        echo "Using DISPLAY: $DISPLAY"
    fi
fi

echo "Executable: $EXECUTABLE"
echo ""
echo "Starting..."
echo "========================================="
echo ""

"$EXECUTABLE"
EXIT_CODE=$?

echo ""
echo "========================================="
echo "  Program exited with code: $EXIT_CODE"
echo "========================================="
echo ""
