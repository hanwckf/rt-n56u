#!/bin/sh

func_start(){
	start-stop-daemon -S -b -x ttyd -- -r 3 -i br0 login
}

func_stop(){
	killall -q ttyd
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
