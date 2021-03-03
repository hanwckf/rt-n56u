#!/bin/bash
# Github: https://github.com/SuzukiHonoka
# Made by starx.

BINARY_NAME="ipt2socks"
BIN_DIR="/usr/bin"
STORAGE_DIR="/etc/storage"
#BINARY_PATH="$STORAGE_DIR/$BINARY_NAME/$BINARY_NAME"
BINARY_PATH="$BIN_DIR/$BINARY_NAME"
IPT2SOCKS_DIR="$STORAGE_DIR/$BINARY_NAME"
CNZONE_FILE="$IPT2SOCKS_DIR/cn.zone"
CHAIN_NAME="IPT2SOCKS"
SET_NAME="china"
LOCK="/tmp/set_lock"
REMOTE_IP="127.0.0.1"

ARG1=$1
ARG2=$2
ARG3=$3

func_save(){
/sbin/mtd_storage.sh save
}

func_start(){
if [ -n "`pidof redsocks`" ]
then
	func_stop
	logger -t $BINARY_NAME "ALREADY RUNNING: KILLED"
fi

if [ -z "$ARG2" ] || [ -z "$ARG3" ]
then
echo "neither server ip nor port can be empty"
return 1
fi

ulimit -n 65535
ipt2socks -s $ARG2 -p $ARG3 -b "0.0.0.0" -l "12345" -R -j 4 &> /dev/null &
logger -t $BINARY_NAME "STARTED."
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
	return 1
fi

func_clean
if [ ! -f $CNZONE_FILE ]
then
	wget -P $IPT2SOCKS_DIR http://www.ipdeny.com/ipblocks/data/countries/cn.zone
	func_save
	logger -t "SET FILE SAVED."
fi

if [ ! -f $LOCK ]
then
	ipset destroy $SET_NAME
	ipset -N $SET_NAME hash:net
	for i in $(cat ${REDSOCKS_DIR}/cn.zone); do ipset -A china $i; done
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
	echo "Usage: $0 {start [IP] [PORT]|stop|iptables [Server IP]|restart}"
	exit 1
	;;
esac
