#!/bin/sh
chnroute_file="/etc/storage/chinadns/chnroute.txt"
bind_address=$(nvram get chinadns_bind)
bind_port=$(nvram get chinadns_port)
server=$(nvram get chinadns_server)
filter=$(nvram get chinadns_bi_filter)

get_arg_filter(){
	if [ "$filter" = "1" ]; then
		echo "-d"
	fi
}

func_start(){
	/usr/bin/check_chnroute
	/usr/bin/enable_update_chnroute
	start-stop-daemon -S -b -x chinadns -- -m -c $chnroute_file -b $bind_address -p $bind_port -s $server $(get_arg_filter)
}

func_stop(){
	killall -q chinadns
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
