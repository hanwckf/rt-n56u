  
#!/bin/sh
# Github: https://github.com/SuzukiHonoka
# Made by starx.
SRC_NAME="haproxy"
DIR_BIN="/usr/bin"
STORAGE_DIR="/etc/storage/$SRC_NAME"
BIN_PATH="$DIR_BIN/$SRC_NAME"
#BIN_PATH="$STORAGE_DIR/$SRC_NAME"
CONF_PATH="$STORAGE_DIR/$SRC_NAME.cfg"
ARG1=$1
ARG2=$2

func_start(){
if [ ! -z "$ARG2" ];then
CONF_PATH=$ARG2
fi
if [ -f "$CONF_PATH" ];then
$BIN_PATH -f $CONF_PATH -p /run/haproxy.pid -S /run/haproxy-master.sock &> /dev/null &
logger -t $SRC_NAME "Started."
else
logger -t $SRC_NAME "CONF_PATH invalid."
fi
}

func_stop(){
killall $SRC_NAME
logger -t $SRC_NAME "Killed."
}

case $ARG1 in
	"start" )
  		func_stop
  		func_start
		;;
	"stop" )
  		func_stop
  		;;
  	* )
  		echo "Usage: start [CONF_PATH] / stop"
esac
