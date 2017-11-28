#!/bin/sh

ss_bin=ss-tunnel
ss_json_file="/tmp/ss.json"

ss_server=$(nvram get ss_server)
ss_server_port=$(nvram get ss_server_port)
ss_method=$(nvram get ss_method)
ss_password=$(nvram get ss_key)

ss_tunnel_remote=$(nvram get ss-tunnel_remote)
ss_tunnel_mtu=$(nvram get ss-tunnel_mtu)
ss_tunnel_local_port=$(nvram get ss-tunnel_local_port)

loger() {
	logger -st $1 $2
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

func_start_ss_tunnel(){
	func_gen_ss_json
	sh -c "$ss_bin -b 0.0.0.0 -u -c $ss_json_file -l $ss_tunnel_local_port --mtu $ss_tunnel_mtu -L $ss_tunnel_remote & > /dev/null 2>&1"
	if [ $? -eq 0 ]; then
		loger $ss_bin "start done"
	else
		loger $ss_bin "start failed"
	fi
}

func_stop(){
	killall -q $ss_bin
}

case "$1" in
start)
	func_start_ss_tunnel
	;;
stop)
	func_stop
	;;
restart)
	func_stop
	func_start_ss_tunnel
	;;
*)
	echo "Usage: $0 { start | stop | restart }"
	exit 1
	;;
esac