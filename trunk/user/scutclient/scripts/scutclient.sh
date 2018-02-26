#!/bin/sh
export PATH='/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin'
echo "scutclient for Padavan V2.2"
username=$2
password=$3
debug=0
hostname=Lenovo-PC
auth_ip=202.38.210.131
version=4472434f4d0096022a
hash=2ec15ad258aee9604b18f2f8114da38db16efd00
scutclient_exec=bin_scutclient

func_log(){
	logger -st "Scutclient" "$1"
}

func_load(){
	nvram set scutclient_username=$username
	nvram set scutclient_password=$password
	nvram set scutclient_debug=$debug
	nvram set scutclient_hostname=$hostname
	nvram set scutclient_server_auth_ip=$auth_ip
	nvram set scutclient_version=$version
	nvram set scutclient_hash=$hash
	nvram set scutclient_done=1
	nvram commit
	func_log "Configuration was successful!"
}
func_check_conf(){
	rm -f /tmp/.modify_etc_storage
	/bin/enable_scutclient_watchcat
	[ -f /tmp/.modify_etc_storage ] && rm -f /tmp/.modify_etc_storage && mtd_storage.sh save > /dev/null 2>&1
}

func_start(){
	if [ "$(nvram get scutclient_done)" != "1" ]; then
		func_log "Please run 'scutclient.sh load <username> <password>' !"
		exit 1
	fi
	if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
		func_log "WAN has no link!"
		exit 1
	fi
	func_check_conf
	func_log "Username : $(nvram get scutclient_username)"
	func_log "Version : $(nvram get scutclient_version)"
	func_log "WAN ip : $(nvram get wan_ipaddr)"
	func_log "WAN mac : $(nvram get wan_hwaddr)"
	func_log "WAN gateway : $(nvram get wan_gateway)"
	func_log "WAN netmask : $(nvram get wan_netmask)"
	func_log "DNS : $(nvram get wan_dns1_x | cut -d ' ' -f 1)"
	echo -n "Starting scutclient:..."
	start-stop-daemon -S -b -x $scutclient_exec -- $(nvram get scutclient_username) $(nvram get scutclient_password) $(nvram get wan_ifname)
	if [ $? -eq 0 ] ; then
		echo "[  OK  ]"
		func_log "Daemon is started"
	else
		echo "[FAILED]"
	fi
}

func_stop(){
	echo -n "Stopping scutclient:..."
	killall -q $scutclient_exec
	$scutclient_exec logoff > /dev/null 2>&1
	echo "[  OK  ]"
	func_log "Stopped"
}

case "$1" in
load)
	if [ -z "$3" ]; then
		func_log "Invalid username or password."
		exit 1
	fi
	func_load
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
