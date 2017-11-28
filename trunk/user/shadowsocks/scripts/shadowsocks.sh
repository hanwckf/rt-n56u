#!/bin/sh

ss_bin="ss-redir"
ss_json_file="/tmp/ss.json"

ss_local_port=$(nvram get ss_local_port)
ss_udp=$(nvram get ss_udp)
ss_server=$(nvram get ss_server)

ss_server_port=$(nvram get ss_server_port)
ss_method=$(nvram get ss_method)
ss_password=$(nvram get ss_key)
ss_mtu=$(nvram get ss_mtu)

ss_mode=$(nvram get ss_mode)
ss_router_proxy=$(nvram get ss_router_proxy)
ss_lower_port_only=$(nvram get ss_lower_port_only)

wan_fw_list=""
wan_fw_ips=""

loger() {
	logger -st $1 $2
}

get_arg_udp() {
	if [ "$ss_udp" = "1" ]; then	
		echo "-u"
	fi
}

get_arg_out(){
	if [ "$ss_router_proxy" = "1" ]; then 
		echo "-o"
	fi
}

get_wan_bp_list(){
	if [ "$ss_mode" = "1" ]; then
		echo "-B /etc/storage/chinadns/chnroute.txt"
	fi
}

get_ipt_ext(){
	if [ "$ss_lower_port_only" = "1" ]; then
		echo '-e "--dport 22:1023"'
	fi
}

func_start_ss_redir(){
	sh -c "$ss_bin -b 0.0.0.0 -c $ss_json_file -l $ss_local_port --mtu $ss_mtu $(get_arg_udp) & > /dev/null 2>&1"
	if [ $? -eq 0 ]; then
		loger $ss_bin "start done"
	else
		loger $ss_bin "start failed"
	fi
}

func_start_ss_rules(){
	ss-rules -f
	sh -c "ss-rules -s $ss_server -l $ss_local_port $(get_wan_bp_list) -W $wan_fw_list -w $wan_fw_ips -d SS_SPEC_WAN_AC $(get_ipt_ext) $(get_arg_out) $(get_arg_udp)"
}

func_gen_ss_json(){
cat > "$ss_json_file" <<EOF
{
    "server": "$ss_server",
    "server_port": "$ss_server_port",
    "password": "$ss_password",
    "method": "$ss_method"
}

EOF
}

func_start(){
	/usr/bin/enable_ss_watchcat
	/usr/bin/enable_update_chnroute
	func_gen_ss_json && func_start_ss_redir && func_start_ss_rules || ss-rules -f
}

func_stop(){
	killall -q $ss_bin
	ss-rules -f
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
