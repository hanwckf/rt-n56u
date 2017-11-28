#!/bin/sh

case "$1" in
start)
        vlmcsd
        logger -st "vlmcsd" "start"
        ;;
stop)
        killall -q vlmcsd
        ;;
restart)
        killall -q vlmcsd
        vlmcsd
        logger -st "vlmcsd" "start"
        ;;
*)
        echo "Usage: $0 { start | stop | restart }"
        exit 1
        ;;
esac
