#!/bin/sh

pidfile="/var/sh_mentohust.pid"
[ -f $pidfile ] && kill -9 "$(cat $pidfile)" || echo "$$" > $pidfile

mentohust_exec="bin_mentohust"
conf_file="/var/mentohust.conf"
bin_conf_file="/etc/storage/mentohust.conf"

mentohust_vars="username password nic ip mask gw dns pinghost timeout interval \
	restart_wait maxfail startmode dhcp daemon ver datafile dhcpscript service"

mentohust_conf_vars="Username Password Nic IP Mask Gateway DNS PingHost Timeout EchoInterval \
	RestartWait MaxFail StartMode DhcpMode DaemonMode Version DataFile DhcpScript Service"

func_log(){
	logger -st "mentohust" "$1"
}

func_gen_conf(){
	if [ -z "$(nvram get mentohust_nic)" ]; then
		nvram set mentohust_nic="$(nvram get wan_ifname)"
		nvram commit
	fi

	echo "[MentoHUST]" > $conf_file
	for c in $mentohust_conf_vars; do
		echo "${c}=" >> $conf_file
	done

	local line=2
	for c in $mentohust_vars; do
		i="$(nvram get mentohust_${c})"
		if [ -n "${i}" ]; then
			i="$(echo $i |sed 's/\//\\\//g')"
			sed -i "${line}s/\$/${i}/" $conf_file
		fi
		line=$((line+1))
	done

	ln -sf $conf_file $bin_conf_file
}

func_start(){
	if [ -f $bin_conf_file ] && [ ! -L $bin_conf_file ]; then
		func_log "Using existed conf: $bin_conf_file"
	else
		func_gen_conf
	fi

	echo -n "Starting mentohust:..."
	start-stop-daemon -S -b -x "$mentohust_exec"
	if [ $? -eq 0 ] ; then
		echo "[  OK  ]"
		func_log "Daemon is started"
	else
		echo "[FAILED]"
	fi
}

func_stop(){
	echo -n "Stopping mentohust:..."
	killall -q -9 $mentohust_exec
	echo "[  OK  ]"
	func_log "Stopped"
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
	;;
esac
rm -f $pidfile
exit 0

