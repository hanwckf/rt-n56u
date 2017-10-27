#!/bin/sh

case "$1" in
start)
        start-stop-daemon -S -b -x ttyd -- -r 3 -i br0 login
        ;;
stop)
        killall ttyd
        ;;
restart)
        killall ttyd
        start-stop-daemon -S -b -x ttyd -- -r 3 -i br0 login
        ;;
*)
        echo "Usage: $0 { start | stop | restart }"
        exit 1
        ;;
esac
