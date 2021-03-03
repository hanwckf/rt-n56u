#!/bin/sh
# Github: https://github.com/SuzukiHonoka
# Made by starx.
SRC_NAME="cf"
DIR_BIN="/usr/bin"
STORAGE_DIR="/etc/storage/$SRC_NAME"
BIN_PATH="$DIR_BIN/$SRC_NAME"
#BIN_PATH="$STORAGE_DIR/$SRC_NAME"
CONF_PATH="$STORAGE_DIR/$SRC_NAME.json"
ARG1=$1
ARG2=$2

func_start(){
if [ ! -z "$ARG2" ];then
CONF_PATH=$ARG2
fi
if [ -f "$CONF_PATH" ];then
$BIN_PATH -conf $CONF_PATH &> /dev/null &
logger -t $SRC_NAME "Started."
else
logger -t $SRC_NAME "CONF_PATH invalid."
fi
}

func_stop(){
killall $SRC_NAME
}

case $ARG1 in
	"start" )
		func_stop
		func_start
		;;
	"stop" )
		func_stop
		;;
	*)
		echo "Usage: start [conf] / stop"
		;;
esac
