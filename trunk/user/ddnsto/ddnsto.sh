#!/bin/sh

start_zero() {
	logger -t "DDNSTO" "正在启动DDNSTO"
	kill_z
	/usr/bin/ddnsto -u "$(nvram get ddnsto_id)" >/dev/null 2>&1 &

}
kill_z() {
	ddnsto_process=$(pidof ddnsto)
	if [ -n "$ddnsto_process" ]; then
		logger -t "DDNSTO" "关闭进程..."
		killall ddnsto >/dev/null 2>&1
		kill -9 "$ddnsto_process" >/dev/null 2>&1
	fi
}
stop_zero() {
	kill_z
	}



case $1 in
start)
	start_zero
	;;
stop)
	stop_zero
	;;
*)
	echo "check"
	#exit 0
	;;
esac
