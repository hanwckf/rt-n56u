#!/bin/sh
bind_address=$(nvram get dns_forwarder_bind)
bind_port=$(nvram get dns_forwarder_port)
server=$(nvram get dns_forwarder_server)

func_start(){
	start-stop-daemon -S -b -x dns-forwarder -- -b "$bind_address" -p "$bind_port" -s "$server"
}

func_stop(){
	killall -q dns-forwarder
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
