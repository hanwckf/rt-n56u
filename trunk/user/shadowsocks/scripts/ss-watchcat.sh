#!/bin/sh

China_ping_domain="www.qq.com"
Foreign_wget_domain="http://www.google.com/"
detect_period=300
log_file="/tmp/ss-watchcat.log"

loger(){
	time=$(date "+%H:%M:%S")
	echo -n "$time " >> $log_file
	logger -st "ss-watchcat" "$1" 2>>$log_file
}

detect_shadowsocks(){
	wget --spider --quiet --timeout=3 $Foreign_wget_domain > /dev/null 2>&1
	[ "$?" = "0" ] && return 0 || return 1
}

restart_apps(){
	/usr/bin/shadowsocks.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart shadowsocks."
	[ -f /usr/bin/dns-forwarder.sh ] && [ "$(nvram get dns_forwarder_enable)" = "1" ] && /usr/bin/dns-forwarder.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart dns-forwarder."
	[ -f /usr/bin/chinadns.sh ] && [ "$(nvram get chinadns_enable)" = "1" ] && /usr/bin/chinadns.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart chinadns."
}

[ "$(pidof ss-watchcat.sh)" != "$$" ] && exit 1

while true; do
	sleep $detect_period
	if [ "$(nvram get ss_watchcat)" != "1" ] || [ "$(nvram get ss_router_proxy)" != "1" ] || [ "$(nvram get ss_enable)" != "1" ]; then
		continue
	fi
	tries=0
	ss_need_restart=1
	while [ $tries -lt 3 ]; do
		if /bin/ping -c 1 $China_ping_domain -W 1 >/dev/null 2>&1 ; then
			detect_shadowsocks
			if [ "$?" = "0" ]; then
				loger "No Problem." && ss_need_restart=0 && break
			elif [ "$?" = "1" ]; then
				tries=$((tries+1))
			fi
		else
			loger "Network Error." && ss_need_restart=0 && break
		fi
	done
	[ $ss_need_restart -eq 1 ] && restart_apps
done
