#!/bin/sh

[ "$(pidof scutclient_watchcat.sh)" != "$$" ] && exit 1
net="202.38.193.188 114.114.114.114 119.29.29.29 202.38.193.33"
net_num=4

while true; do
	sleep 60
	if [ "$(nvram get scutclient_watchcat)" != "1" ] || [ "$(nvram get scutclient_enable)" != "1" ]; then
		continue
	fi

	if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
		logger -st "scutclient_watchcat" "WAN has no link!"
		continue
	fi

	fails=0
	for n in $net; do
		tries=0
		while [ $tries -lt 3 ]; do
			if /bin/ping -c 1 "$n" -W 1 >/dev/null ; then
				break 2
			else
				tries=$((tries+1))
			fi
		done
		fails=$((fails+1))
	done
	[ $fails -eq $net_num ] && \
	logger -st "scutclient_watchcat" "Connection failed, restart scutclient!" && \
	/bin/scutclient.sh restart
done
