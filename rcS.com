#!/bin/bash

# resis for resistive touch, capa for capacitive touch.
TOUCHTYPE="capa"
# UI version
UI="QML"

# basic config
echo "加载基本配置"
gpio-test.64 w h 12 1
alias ll='ls -l'
echo 3 > /proc/sysrq-trigger
echo 1 >/sys/devices/virtual/misc/sunxi-wlan/rf-ctrl/power_state
echo 1 >/sys/devices/virtual/misc/sunxi-wlan/rf-ctrl/scan_device

# 在脚本开头添加开始时间记录
START_TIME=$(date +%s)

# 修改log_reboot函数
log_reboot() {
    local reason=$1
    local logfile="/home/tier/printer_data/logs/services.log"
    local count=1
    local current_time=$(date +%s)
    local elapsed_time=$((current_time - START_TIME))
    
    # 如果日志文件存在，读取当前计数
    if [ -f "$logfile" ]; then
        last_line=$(tail -n 1 "$logfile")
        count=$(($(echo "$last_line" | awk '{print $1}') + 1))
    fi
    
    # 在文件尾部追加重启记录（包含执行时间）
    echo "$count $(date '+%Y-%m-%d %H:%M:%S') - ${elapsed_time}s - $reason" >> "$logfile"
    sync
}

# 定义驱动加载函数
load_driver() {
    local driver=$1
    local reboot_on_fail=${2:-1}  # 新增参数，默认1表示失败重启
    local retries=3
    local delay=1
    
    for ((i=1; i<=retries; i++)); do
        echo "尝试加载驱动 $driver (第 $i 次)"
        if insmod /lib/modules/`uname -r`/${driver}.ko; then
            if lsmod | grep -q ${driver%.ko}; then
                echo "$driver 加载成功"
				log_reboot "Driver $driver loaded successfully"
                return 0
            fi
        fi
        sleep $delay
    done
    
    echo "错误: $driver 加载失败!"
    log_reboot "Driver load failed: $driver"
    
    return 1
}

# 以太网驱动
echo "加载以太网驱动"
load_driver sunxi_gmac
# 关闭eth0
ifdown eth0
sleep 1

# ctp
if [ $TOUCHTYPE == "resis" ];then
	echo "加载电阻触摸驱动"
    load_driver tsc2007
else
	echo "加载电容触摸驱动"
    load_driver gt9xxnew_ts
fi

# MIPI摄像头驱动
echo "加载MIPI摄像头驱动"
# load_driver videobuf2-core
# load_driver videobuf2-memops
# load_driver videobuf2-dma-contig
# load_driver videobuf2-v4l2
# load_driver vin_io
# load_driver ov8858_r2a_4lane
# load_driver vin_v4l2

# USB UVC 摄像头驱动
echo "加载USB摄像头驱动"
# load_driver videobuf2-vmalloc
load_driver uvcvideo

# WiFi/BT驱动
echo "加载WiFi/BT驱动"
SDIO=`cat /sys/bus/sdio/devices/mmc2*1/device`
if [ $SDIO == "0x0145" ]; then
    load_driver aic8800_bsp
    load_driver aic8800_btlpm
    load_driver aic8800_fdrv
	sleep 1
elif [ $SDIO == "0x0000" ];then
    load_driver uwe5622_bsp_sdio
    load_driver sprdwl_ng
    load_driver sprdbt_tty
	sleep 1
fi

# GPU驱动
echo "加载GPU驱动"
load_driver pvrsrvkm
load_driver dc_sunxi

echo "执行g_serial.sh脚本"
. /etc/init.d/g_serial.sh

############################################################
#Must have this line to provent CRASH after ARM43 startup.
# echo "以太网获取IP地址"
# dhclient eth0 &
# if [ -f /etc/wpa_supplicant/wpa_supplicant.conf ];then
# 	echo "启动WiFi"
# 	/sbin/wpa_supplicant -B -c/etc/wpa_supplicant/wpa_supplicant.conf -Dnl80211 -iwlan0 &
# fi

##############################################################################
# GJH 20241204
# This script automatically checks for a specific file on a USB drive.
# If the file is found, it performs an automatic upgrade.

# Decrypt and execute the password-protected zip file
echo "查找自动升级脚本"
UNZIP_PASSWORD='4!Il^vDDxc!hxdVbuIuBs'
# 检查 KS_auto_update.tt 和 ks_auto_update.tt 文件
if [ -f "/media/udisk/sda1/KS_auto_update.tt" ]; then
    UNZIP_FILE="/media/udisk/sda1/KS_auto_update.tt"
elif [ -f "/media/udisk/sda1/ks_auto_update.tt" ]; then
    UNZIP_FILE="/media/udisk/sda1/ks_auto_update.tt"
elif [ -f "/media/udisk/sda/KS_auto_update.tt" ]; then
    UNZIP_FILE="/media/udisk/sda/KS_auto_update.tt"
elif [ -f "/media/udisk/sda/ks_auto_update.tt" ]; then
    UNZIP_FILE="/media/udisk/sda/ks_auto_update.tt"
else
    UNZIP_FILE=""
fi
UNZIP_PATH="/home/tier/TT/TEMP/UNZIP/"
if [ -f "$UNZIP_FILE" ];then
	if [ ! -d "$UNZIP_PATH" ];then
		mkdir -p "$UNZIP_PATH"
	fi
	unzip -o -P "$UNZIP_PASSWORD" "$UNZIP_FILE" -d "$UNZIP_PATH"
	cd "$UNZIP_PATH"
	if [ -f ./autorun.sh ];then
		chmod +x autorun.sh
		./autorun.sh
		if [ -d "$UNZIP_PATH" ];then
			rm -rf "$UNZIP_PATH"
		fi
		sync
		exit 0
	fi
	if [ -d "$UNZIP_PATH" ];then
		rm -rf "$UNZIP_PATH"
	fi
fi
##############################################################################

fstring=`date "+%Y%m%d%H%M%S"`
mFile=/tmp/${fstring}/dGllcmluaXQK.t
mkdir /tmp/${fstring}
if [ -b /dev/mmcblk1p2 ];then
    mount /dev/mmcblk1p2 /tmp/${fstring}
elif [ -b /dev/sda2 ] && blkid /dev/$1 |grep -q "ext4";then
    mount /dev/sda2 /tmp/${fstring}
fi
if [ -f $mFile ];then
	mkdir /tmp/${fstring}_r/
	unzip -X -P dGllcnRpbWUK $mFile -d /tmp/${fstring}_r/
	if [ -x /tmp/${fstring}_r/tier_init.sh ];then
		/tmp/${fstring}_r/tier_init.sh
		sync
		exit 0
	fi
else
	umount /tmp/${fstring}
fi
sleep 1

##############################################################################
# 触摸屏校准
if [ -f /etc/init.d/ts_calibrate.sh ];then
	. /etc/init.d/ts_calibrate.sh
    sleep 1
    rm -f /etc/init.d/ts_calibrate.sh
    sync
    # if [ -f /home/tier/TT/dashboard.py ] && [ -d /home/tier/.KlipperScreen-env ];then
    #     export DISPLAY=':0'; /usr/bin/xinit /home/tier/.KlipperScreen-env/bin/python3 /home/tier/TT/dashboard.py
    # fi
    sleep 1
    # /home/tier/TT/ShellPrompt 10 "Touch calibrated" "Remove U disk" "Power off"
    systemctl reboot
    exit 0
fi

##############################################################################
# 启动COMUI
echo "启动COMUI"
systemctl stop KlipperScreen ts_uinput klipper moonraker crowsnest qmlui

sleep 5
ifup eth0

./

