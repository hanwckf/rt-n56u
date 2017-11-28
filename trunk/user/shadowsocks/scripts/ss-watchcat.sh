#!/bin/sh

LOGTIME=$(date "+%H:%M:%S")

net_domain="www.qq.com"
log_file="/tmp/ss-watchcat.log"

loger(){
	echo -n "$LOGTIME " >> /tmp/ss-watchcat.log
	logger -st "ss-watchcat" "$1" 2>>$log_file
}

detect_shadowsocks(){
wget --spider --quiet --timeout=3 http://www.google.com/ > /dev/null 2>&1
if [ "$?" = "0" ]; then
	echo "$LOGTIME ss-watchcat: No Problem." >> $log_file
	exit 0
else
	loger "Problem decteted, restarting shadowsocks."
	/usr/bin/shadowsocks.sh restart >/dev/null 2>&1
	#/usr/bin/dns-forwarder.sh restart >/dev/null 2>&1
	#/usr/bin/chinadns.sh restart >/dev/null 2&1
fi
}

if [ "$(nvram get ss_watchcat)" != "1" ] || [ "$(nvram get ss_router_proxy)" != "1" ] || [ "$(nvram get ss_enable)" != "1" ]; then
	exit 0
fi

if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
	loger "WAN has no link!"
	exit 1
fi

tries=0
while [[ $tries -lt 3 ]]
do
	if /bin/ping -c 1  $net_domain -W 1 >/dev/null
	then
		detect_shadowsocks
		exit 0
	fi
tries=$((tries+1))
done
