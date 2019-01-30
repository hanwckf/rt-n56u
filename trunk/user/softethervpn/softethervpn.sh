#!/bin/sh

vpn_conf="/etc/storage/softethervpn"
vpn_ro_dir="/etc_ro/softethervpn"
vpn_exec_dir="/tmp/softethervpn"

func_log(){
	logger -st "softethervpn" "$1"
}

func_start(){
	[ ! -f $vpn_ro_dir/$1 ] && echo "$1 not found!" && exit 1
	mkdir -p $vpn_exec_dir
	[ -f $vpn_conf/vpn_client.config ] && cp -f $vpn_conf/vpn_client.config $vpn_exec_dir/vpn_client.config
	[ -f $vpn_conf/vpn_server.config ] && cp -f $vpn_conf/vpn_server.config $vpn_exec_dir/vpn_server.config
	[ ! -f $vpn_exec_dir/$1 ] && ln -sf $vpn_ro_dir/$1 $vpn_exec_dir/$1
	[ ! -f $vpn_exec_dir/hamcore.se2 ] && ln -sf $vpn_ro_dir/hamcore.se2 $vpn_exec_dir/hamcore.se2
	if [ "$1" = "vpncmd" ]; then
		LANG=en_US.UTF-8 $vpn_exec_dir/$1
	else
		LANG=en_US.UTF-8 $vpn_exec_dir/$1 start && func_log "$1 Started"
	fi
}

func_stop(){
	[ -f $vpn_exec_dir/$1 ] && LANG=en_US.UTF-8 $vpn_exec_dir/$1 stop && func_log "$1 Stopped"
}

func_save(){
	mkdir -p $vpn_conf
	[ -f $vpn_exec_dir/vpn_client.config ] && cp -f $vpn_exec_dir/vpn_client.config $vpn_conf/vpn_client.config
	[ -f $vpn_exec_dir/vpn_server.config ] && cp -f $vpn_exec_dir/vpn_server.config $vpn_conf/vpn_server.config
	mtd_storage.sh save
}

case "$1" in
save)
		func_save
	;;
cmd)
		func_start vpncmd
	;;
start_srv)
		func_start vpnserver
	;;
start_cli)
		func_start vpnclient
	;;
stop_srv)
		func_stop vpnserver
	;;
stop_cli)
		func_stop vpnclient
	;;
*)
		echo "Usage: $0 { cmd | start_srv | start_cli | stop_srv | stop_cli | save }"
		exit 1
	;;
esac
