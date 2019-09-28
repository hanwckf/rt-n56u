#!/bin/sh
nvram_ttyd_port="$(nvram get ttyd_port)"
[ -z "$nvram_ttyd_port" ] && nvram set ttyd_port=7681 && nvram commit
port=${nvram_ttyd_port:-"7681"}

func_start(){
	start-stop-daemon -S -b -x ttyd -- -i br0 -p "$port" login
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
