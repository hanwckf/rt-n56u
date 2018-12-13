#!/bin/sh

pidfile="/var/ss-watchcat.pid"
China_ping_domain="www.qq.com"
Foreign_wget_domain="http://www.google.com/"
log_file="/tmp/ss-watchcat.log"
max_log_bytes=100000

go_exit(){
	rm -f $pidfile
	exit
}

loger(){
	[ -f $log_file ] && [ $(stat -c %s $log_file) -gt $max_log_bytes ] && rm -f $log_file
	time=$(date "+%H:%M:%S")
	echo "$time ss-watchcat $1" >> $log_file
}

detect_shadowsocks(){
	wget --spider --quiet --timeout=3 $Foreign_wget_domain > /dev/null 2>&1
	return $?
}

restart_apps(){
	/usr/bin/shadowsocks.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart shadowsocks."
	[ -f /usr/bin/dns-forwarder.sh ] && [ "$(nvram get dns_forwarder_enable)" = "1" ] && /usr/bin/dns-forwarder.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart dns-forwarder."
}

[ -f $pidfile ] && kill -9 "$(cat $pidfile)" || echo "$$" > $pidfile

if [ "$(nvram get ss_watchcat)" != "1" ] || [ "$(nvram get ss_router_proxy)" != "1" ] || [ "$(nvram get ss_enable)" != "1" ]; then
	go_exit
fi

tries=0
while [ $tries -lt 3 ]; do
	detect_shadowsocks
	if [ "$?" = "0" ]; then
		loger "No Problem."
		go_exit
	else
		tries=$((tries+1))
	fi
done
/bin/ping -c 3 $China_ping_domain -w 5 >/dev/null 2>&1
[ "$?" = 1 ] && loger "Network Error." && go_exit
restart_apps
go_exit
