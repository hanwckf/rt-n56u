#!/bin/sh

LOGTIME=$(date "+%H:%M:%S")

net_domain="www.qq.com"
log_file="/tmp/ss-watchcat.log"

loger(){
	echo -n "$LOGTIME " >> $log_file
	logger -st "ss-watchcat" "$1" 2>>$log_file
}

detect_shadowsocks(){
	wget --spider --quiet --timeout=3 http://www.google.com/ > /dev/null 2>&1
	if [ "$?" = "0" ]; then
		loger "No Problem."
		exit 0
	else
		/usr/bin/shadowsocks.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart shadowsocks."
		[ -f /usr/bin/dns-forwarder.sh ] && /usr/bin/dns-forwarder.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart dns-forwarder."
		[ -f /usr/bin/chinadns.sh ] && /usr/bin/chinadns.sh restart >/dev/null 2>&1 && loger "Problem decteted, restart chinadns."
	fi
}

if [ "$(nvram get ss_watchcat)" != "1" ] || [ "$(nvram get ss_router_proxy)" != "1" ] || [ "$(nvram get ss_enable)" != "1" ]; then
	exit 0
fi

tries=0
while [ $tries -lt 3 ]
do
	if /bin/ping -c 1  $net_domain -W 1 >/dev/null
	then
		detect_shadowsocks
		exit 0
	fi
tries=$((tries+1))
done
