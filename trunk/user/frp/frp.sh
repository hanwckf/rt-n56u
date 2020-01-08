#!/bin/sh
frpc_enable=`nvram get frpc_enable`
frps_enable=`nvram get frps_enable`

frp_start () {
eval "/etc/storage/frp_script.sh" &
restart_dhcpd
[ ! -z "`pidof frpc`" ] && logger -t "frp" "frpc启动成功"
[ ! -z "`pidof frps`" ] && logger -t "frp" "frps启动成功"
}

frp_close () {
killall frpc frps frp_script.sh
killall -9 frpc frps frp_script.sh
restart_dhcpd
[ -z "`pidof frpc`" ] && logger -t "frp" "停止 frpc"
[ -z "`pidof frps`" ] && logger -t "frp" "停止 frps"
}


case $1 in
start)
	frp_close
	frp_start
	;;
stop)
	frp_close
	;;
*)

	;;
esac
