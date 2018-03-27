#!/bin/sh
scutclient_exec="bin_scutclient"
dns=$(nvram get wan_dns1_x | cut -d ' ' -f 1)
[ -z "$(nvram get scutclient_auth_exec)" ] && nvram set scutclient_auth_exec="echo 0 > /tmp/scutclient_status"
[ -z "$(nvram get scutclient_fail_exec)" ] && nvram set scutclient_fail_exec="echo 1 > /tmp/scutclient_status"

func_log(){
	logger -st "Scutclient" "$1"
}

func_load(){
	nvram set scutclient_username="$1"
	nvram set scutclient_password="$2"
	nvram set scutclient_debug=0
	nvram set scutclient_hostname="Lenovo-PC"
	nvram set scutclient_server_auth_ip="202.38.210.131"
	nvram set scutclient_version="4472434f4d0096022a"
	nvram set scutclient_hash="2ec15ad258aee9604b18f2f8114da38db16efd00"
	nvram set scutclient_done=1
	nvram commit
	func_log "init done!"
}

get_arg_debug(){
	[ "$(nvram get scutclient_debug)" = "1" ] && echo "-D"
}

func_check_watchcat(){
	rm -f /tmp/.modify_etc_storage
	/bin/enable_scutclient_watchcat
	[ -f /tmp/.modify_etc_storage ] && rm -f /tmp/.modify_etc_storage && mtd_storage.sh save > /dev/null 2>&1
}

func_start(){
	[ "$(nvram get scutclient_done)" != "1" ] && func_log "Please run 'scutclient.sh load <username> <password>' !" && exit 1
	[ "$(mtk_esw 11)" = "WAN ports link state: 0" ] && func_log "WAN has no link!" && exit 1
	func_check_watchcat
	func_log "Username : $(nvram get scutclient_username)"
	func_log "Version : $(nvram get scutclient_version)"
	func_log "WAN ip : $(nvram get wan_ipaddr)"
	func_log "WAN gateway : $(nvram get wan_gateway)"
	func_log "WAN netmask : $(nvram get wan_netmask)"
	echo -n "Starting scutclient:..."
	start-stop-daemon -S -b -x "$scutclient_exec" -- -u "$(nvram get scutclient_username)" -p "$(nvram get scutclient_password)" \
	-f "$(nvram get wan_ifname)" \
	-n "$dns" \
	-t "$(nvram get scutclient_hostname)" \
	-s "$(nvram get scutclient_server_auth_ip)" \
	-c "$(nvram get scutclient_version)" \
	-h "$(nvram get scutclient_hash)" \
	-E "$(nvram get scutclient_auth_exec)" \
	-F "$(nvram get scutclient_fail_exec)" \
	"$(get_arg_debug)"

	if [ $? -eq 0 ] ; then
		echo "[  OK  ]"
		func_log "Daemon is started"
	else
		echo "[FAILED]"
	fi
}

func_stop(){
	echo -n "Stopping scutclient:..."
	killall -q -9 $scutclient_exec
	$scutclient_exec -o -f "$(nvram get wan_ifname)" >/dev/null 2>&1
	echo "[  OK  ]"
	func_log "Stopped"
}

case "$1" in
load)
	if [ -z "$3" ]; then
		func_log "Invalid username or password."
		exit 1
	fi
	func_load "$1" "$2"
	func_start
	;;
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
	echo "Usage: $0 { load <username> <password> | start | stop | restart }"
	exit 1
	;;
esac
