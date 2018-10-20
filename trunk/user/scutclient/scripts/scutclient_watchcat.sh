#!/bin/sh

[ "$(pidof scutclient_watchcat.sh)" != "$$" ] && exit 1
net="202.38.193.188 114.114.114.114 119.29.29.29"
net_num=3

while true; do
	sleep 60
	if [ "$(nvram get scutclient_watchcat)" != "1" -o "$(nvram get scutclient_enable)" != "1" ]; then
		continue
	fi

	if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
		logger -st "scutclient_watchcat" "WAN has no link!"
		continue
	fi

	fails=0
	for n in $net; do
		/bin/ping -c 3 "$n" -w 5 >/dev/null 2>&1
		if [ "$?" = "0" ]; then
			break
		else
			fails=$((fails+1))
		fi
	done
	[ $fails -eq $net_num ] && \
	logger -st "scutclient_watchcat" "Connection failed, restart scutclient!" && \
	/bin/scutclient.sh restart
done
