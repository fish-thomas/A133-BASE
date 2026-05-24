# HelperBoard A133 RGB Display Demo

A Qt5-based UI application for HelperBoard A133 with RGB display support.

## Features

- **Real-time Clock Display** - Shows current date and time
- **WiFi Settings** - Configure and manage WiFi connections
- **Embedded Terminal** - Built-in terminal emulator for command execution
- **Animation Demo** - Colorful moving rectangle animation
- **Touch-friendly UI** - Optimized for 800x480 touchscreen displays

## Project Structure

```
rgb_display_demo/
├── main.cpp              # Application entry point
├── mainwindow.h/cpp      # Main window with clock, buttons, terminal
├── wifidialog.h/cpp      # WiFi configuration dialog
├── terminalwidget.h/cpp # Terminal emulator widget
├── rgb_display_demo.pro  # Qt project file
└── README.md             # This file
```

## Requirements

- Qt 5.15 or higher
- HelperBoard A133 or compatible ARM development board
- Linux framebuffer support

## Building

### On HelperBoard A133

```bash
cd rgb_display_demo
rm -rf build
mkdir build && cd build
qmake ../rgb_display_demo.pro
make
```

### Using build script (Recommended)

```bash
# Copy build.sh to the board
chmod +x build.sh
./build.sh
```

## Running

### Using run script (Recommended)

```bash
chmod +x run.sh
./run.sh
```

### Manual

```bash
cd ~
export QT_QPA_PLATFORM=linuxfb
export QT_QPA_EGLFS_WIDTH=800
export QT_QPA_EGLFS_HEIGHT=480
./rgb_display_demo
```

## Usage

### Main Window
- **Title Area**: Shows "HelperBoard A133 RGB Display Demo"
- **Time Display**: Shows current system time
- **WiFi Settings Button**: Opens WiFi configuration dialog
- **Terminal Mode Button**: Opens full-screen terminal
- **Input Field**: Enter text to display
- **Update Display Button**: Updates the display label with input text
- **Animation Buttons**: Start/Stop moving rectangle animation
- **Terminal Widget**: Embedded terminal for command execution

### WiFi Dialog
- **Scan WiFi**: Scans for available networks
- **Network List**: Shows available WiFi networks with signal strength
- **SSID Input**: Enter or select WiFi network name
- **Password Input**: Enter WiFi password
- **Connect/Disconnect**: Connect to or disconnect from WiFi
- **Status Display**: Shows current connection status

### Terminal Emulator
Supports common commands:
- `help` - Show available commands
- `clear` / `cls` - Clear screen
- `pwd` - Print working directory
- `cd <dir>` - Change directory
- `ls` - List directory contents
- `date` - Show current date/time
- `whoami` - Show current user
- `hostname` - Show hostname
- `uname` - Show system information
- `ifconfig` - Show network interfaces
- `top` - Show system processes
- `uptime` - Show system uptime
- `free` - Show memory usage
- `df` - Show disk usage
- `echo <msg>` - Print message

## Environment Variables

The following environment variables may need to be configured:

```bash
export QT_QPA_PLATFORM=linuxfb          # Framebuffer platform
export QT_QPA_EGLFS_WIDTH=800           # Screen width
export QT_QPA_EGLFS_HEIGHT=480          # Screen height
```

## Troubleshooting

### Platform plugin error
If you see "Could not load the Qt platform plugin", make sure to:
1. Clear all Qt environment variables
2. Set QT_QPA_PLATFORM=linuxfb

### Touch screen issues
Make sure TSLIB is properly configured on your system.

## License

This project is provided as-is for use with HelperBoard A133.

## Author

Created for HelperBoard A133 RGB Display Demo
