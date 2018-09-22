#!/bin/sh

[ "$(pidof scutclient_watchcat.sh)" != "$$" ] && exit 1
net="www.qq.com www.baidu.com www.taobao.com"
net_num=3

while true; do
	sleep 30
	if [ "$(nvram get scutclient_watchcat)" != "1" ] || [ "$(nvram get scutclient_enable)" != "1" ]; then
		continue
	fi

	if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
		logger -st "scutclient_watchcat" "WAN has no link!"
		continue
	fi

	fails=0
	for n in $net; do
		if /bin/ping -c 1 "$n" -W 1 >/dev/null ; then
			break
		else
			fails=$((fails+1))
		fi
	done
	[ $fails -eq $net_num ] && logger -st "scutclient_watchcat" "Connection failed, restart scutclient!" && /bin/scutclient.sh restart
done
