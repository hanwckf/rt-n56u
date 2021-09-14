# 准备打印机热插拔时安装固件(从网上下载固件)
cat > "/var/usblp_hotplug.sh" <<-\EOF
#!/bin/sh
set -e
HPLJSITE=http://oleg.wl500g.info/hplj
LOGFILE=/var/usblp_hotplug.log
FIRMWARE=
if [ $# -eq 3 ]; then
    #这里用于开机时调用
    DEVNAME=$1
    ACTION=$2
    DEVD=$3/../../..
else
    DEVD=/sys${DEVPATH}/../../..
fi
if [ -f $DEVD/product ]; then
    product=`cat $DEVD/product`
    vid=`cat $DEVD/idVendor`
    pid=`cat $DEVD/idProduct`
    case $vid-$pid in
    03f0-0517)
        FIRMWARE=sihp1000.dl
        ;;
    03f0-1317)
        FIRMWARE=sihp1005.dl
        ;;
    03f0-4117)
        FIRMWARE=sihp1018.dl
        ;;
    03f0-2b17)
        FIRMWARE=sihp1020.dl
        ;;
    03f0-3d17)
        FIRMWARE=sihpP1005.dl
        ;;
    03f0-3e17)
        FIRMWARE=sihpP1006.dl
        ;;
    03f0-4817)
        FIRMWARE=sihpP1005.dl
        ;;
    03f0-4917)
        FIRMWARE=sihpP1006.dl
        ;;
    03f0-3f17)
        FIRMWARE=sihpP1505.dl
        ;;
    esac
    if [ $FIRMWARE ]; then
        if [ ! -f /var/$FIRMWARE ]; then
            curl -o /var/$FIRMWARE $HPLJSITE/$FIRMWARE
        fi
        if [ -c /dev/$DEVNAME -a $ACTION = 'add' ]; then
           echo "$(date "+%Y-%m-%d %H:%M:%S") : Sending $product firmware to $DEVNAME" > $LOGFILE
           cat /var/$FIRMWARE > /dev/$DEVNAME
           echo "$(date "+%Y-%m-%d %H:%M:%S") : done." >> $LOGFILE
        fi
    fi
fi
#调用Padavan原处理程序
if [ $# -eq 2 ]; then
  /sbin/mdev_lp $MDEV $ACTION
fi
EOF
chmod a+x /var/usblp_hotplug.sh
sed -i 's/\/sbin\/mdev_lp/\/var\/usblp_hotplug.sh/' /etc/mdev.conf
# 启动时如果检查到了打印机，就安装固件
if [ -c /dev/usb/lp0 ]; then
    devpath=`find /sys/devices/platform/ehci-platform/ -name 'lp0' | grep 'usb/lp0'`
    if [ $? = 0 ]; then
        /var/usblp_hotplug.sh usb/lp0 add $devpath
    fi
fi