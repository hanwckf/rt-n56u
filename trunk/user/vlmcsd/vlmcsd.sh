#!/bin/sh

func_start(){
	vlmcsd
	logger -st "vlmcsd" "start"
}

func_stop(){
	killall -q vlmcsd
}

case "$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
restart)
	func_stop
	func_start
	;;
*)
	echo "Usage: $0 { start | stop | restart }"
	exit 1
	;;
esac
