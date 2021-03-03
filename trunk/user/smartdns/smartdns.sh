#!/bin/sh
# Github: https://github.com/SuzukiHonoka
# Made by starx.
SRC_NAME="smartdns"
DIR_BIN="/usr/bin"
STORAGE_DIR="/etc/storage/$SRC_NAME"
BIN_PATH="$DIR_BIN/$SRC_NAME"
#BIN_PATH="$STORAGE_DIR/$SRC_NAME"
CONF_PATH="$STORAGE_DIR/$SRC_NAME.conf"
ARG1=$1
ARG2=$2
func_save(){
/sbin/mtd_storage.sh save
}

func_start(){
if [ ! -f "$CONF_PATH" ];then
if [ ! -d "$STORAGE_DIR" ];then
mkdir -p $STORAGE_DIR
fi
cat >> $CONF_PATH << EOF
server-name smartdns
bind [::]:5354
cache-size 4096
prefetch-domain yes
speed-check-mode tcp:443,ping
dualstack-ip-selection yes
server 119.29.29.29
server 119.28.28.28
server 223.5.5.5
server 223.6.6.6
server 114.114.114.114
server 114.114.115.115
server 180.76.76.76
server-tcp 119.29.29.29
server-tcp 119.28.28.28
server-tcp 223.5.5.5
server-tcp 223.6.6.6
server-tcp 114.114.114.114
server-tcp 114.114.115.115
server-tcp 180.76.76.76
EOF
func_save
logger -t $SRC_NAME "Configuration written"
fi
if [ -n "`pidof $SRC_NAME`" ];then
func_stop
logger -t $SRC_NAME "Already running, killed"
fi
$BIN_PATH -f -c $CONF_PATH &> /dev/null &
logger -t $SRC_NAME "Started"
}

func_stop(){
killall $SRC_NAME
logger -t $SRC_NAME "Killed"
}

func_restart(){
func_stop
func_start
}

func_iptables(){
func_ioff
iptables -t nat -A PREROUTING -p tcp --dport 53 -j REDIRECT --to-port 5354
iptables -t nat -A PREROUTING -p udp --dport 53 -j REDIRECT --to-port 5354
logger -t $SRC_NAME "iptables setup done."
}

func_ioff(){
iptables -t nat -D PREROUTING `iptables -t nat -L PREROUTING --line-numbers|grep 5354|head -n 1|tr -cd [1-9]|sed "s/5354//g"`
iptables -t nat -D PREROUTING `iptables -t nat -L PREROUTING --line-numbers|grep 5354|head -n 1|tr -cd [1-9]|sed "s/5354//g"`
logger -t $SRC_NAME "iptables cleaned."
}

func_dnsmasq(){
func_doff
cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
no-resolv
server=127.0.0.1#5354
EOF
/sbin/restart_dhcpd
logger -t $SRC_NAME "dnsmasq setup done."
}

func_doff(){
sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
/sbin/restart_dhcpd
logger -t $SRC_NAME "dnsmasq cleaned."
}

func_setup(){
case $ARG2 in
	"dnsmasq")
		func_dnsmasq
		;;
	"iptables")
		func_iptables
		;;
	*)
		echo "Please specify the setup method"
		;;
esac
}

func_destory(){
	case $ARG2 in
		"iptables")
			func_ioff
			;;
		"dnsmasq")
			func_doff
			;;
		*)
			echo "Please specify the setup method"
			;;		
	esac
}

case $ARG1 in
	"start")
		func_start
		;;
	"stop")
		func_stop
		;;
	"setup")
		func_setup
		;;
	"destory")
		func_destory
		func_stop
		;;
	"restart")
		func_restart
		;;
	*)
		echo "Usage: start/ stop / setup [mode] / destory [mode] / restart"		
		;;
esac
