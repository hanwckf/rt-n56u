#!/bin/bash
# Github: https://github.com/SuzukiHonoka
# Made by starx.

BINARY_NAME="redsocks"
BIN_DIR="/usr/bin"
STORAGE_DIR="/etc/storage"
#BINARY_PATH="$STORAGE_DIR/$BINARY_NAME/$BINARY_NAME"
BINARY_PATH="$BIN_DIR/$BINARY_NAME"
REDSOCKS_DIR="$STORAGE_DIR/$BINARY_NAME"
REDSOCKS_CONF="$REDSOCKS_DIR/$BINARY_NAME.conf"
CNZONE_FILE="$REDSOCKS_DIR/cn.zone"
CHAIN_NAME="REDSOCKS"
SET_NAME="china"
LOCK="/tmp/set_lock"
MODE=false

SOCKS5_IP="192.168.31.99"
SOCKS5_PORT="10808"
REMOTE_IP="127.0.0.1"

ARG1=$1
ARG2=$2
ARG3=$3

func_redsocks(){
cd $REDSOCKS_DIR
$BINARY_PATH $REDSOCKS_CONF
}

func_save(){
/sbin/mtd_storage.sh save
}

func_start(){
MODE=true
if [ -n "`pidof redsocks`" ]
then
	func_stop
	logger -t $BINARY_NAME "ALREADY RUNNING: KILLED"
fi
if [ ! -f $REDSOCKS_CONF ]
then
	if  [ -n "$ARG2" ]
	then
		if [ -n "$ARG3" ]
		then
			SOCKS5_IP=$ARG2
			SOCKS5_PORT=$ARG3
			if [ ! -d $REDSOCKS_DIR ]
			then
			mkdir -p $REDSOCKS_DIR	
			fi
		cat > $REDSOCKS_CONF <<EOF
base {
log_debug = off;
log_info = off;
redirector = iptables;
daemon = on;
redsocks_conn_max = 10000;
rlimit_nofile = 10240;
}
redsocks {
local_ip = 0.0.0.0;
local_port = 12345;
ip = $SOCKS5_IP;
port = $SOCKS5_PORT;
type = socks5;
}
EOF
func_save
logger -t $BINARY_NAME "CONFIG FILE SAVED."
func_redsocks
fi
else
	logger -t $BINARY_NAME "CONFIG FILE NOT FOUND"
	return 0
fi
else
	func_redsocks
	logger -t $BINARY_NAME "STARTED."
fi
}

func_clean(){
iptables -t nat -D PREROUTING `iptables -t nat -L PREROUTING --line-numbers|grep $CHAIN_NAME|head -n 1|tr -cd "[0-9]"`
iptables -t nat -D OUTPUT `iptables -t nat -L OUTPUT --line-numbers|grep $CHAIN_NAME|head -n 1|tr -cd "[0-9]"`
iptables -t nat -F $CHAIN_NAME
iptables -t nat -X $CHAIN_NAME
}

func_iptables(){
if [ -n "$ARG2" ]
then
	REMOTE_IP=$ARG2
else
	logger -t $BINARY_NAME "REMOTE_IP NOT FOUND!"
	return 0
fi

func_clean

if [ ! -f $CNZONE_FILE ]
then
	wget -P $REDSOCKS_DIR http://www.ipdeny.com/ipblocks/data/countries/cn.zone
	func_save
	logger -t "SET FILE SAVED."
fi

if [ ! -f $LOCK ]
then
	ipset destroy $SET_NAME
	ipset -N $SET_NAME hash:net
	for i in $(cat ${REDSOCKS_DIR}/cn.zone ); do ipset -A china $i; done
	touch $LOCK
	logger -t $BINARY_NAME "SET LOCKED"
fi

iptables -t nat -N $CHAIN_NAME
iptables -t nat -A $CHAIN_NAME -d $REMOTE_IP -j RETURN
iptables -t nat -A $CHAIN_NAME -d 0.0.0.0/8 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 10.0.0.0/8 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 127.0.0.0/8 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 169.254.0.0/16 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 172.16.0.0/12 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 192.168.0.0/16 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 224.0.0.0/4 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 240.0.0.0/4 -j RETURN
iptables -t nat -A $CHAIN_NAME -m set --match-set china dst -j RETURN
iptables -t nat -A $CHAIN_NAME -p tcp -j REDIRECT --to-ports 12345 
iptables -t nat -A PREROUTING -p tcp -j $CHAIN_NAME
iptables -t nat -A OUTPUT -p tcp -j $CHAIN_NAME
}

func_stop(){
killall $BINARY_NAME
logger -t $BINARY_NAME "KILLED"
}

case "$ARG1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
iptables)
	func_iptables
	;;
clean)
	func_clean
	;;
restart)
	func_stop
	func_start
	;;
*)
	echo "Usage: $0 {start [ ARG2:Server IP ARG3:Server PORT ]|stop|iptables [ ARG1:Server IP ]|restart}"
	exit 1
	;;
esac