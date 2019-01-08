#!/bin/sh
pidfile="/var/scutclient_watchcat.pid"
net="202.38.193.188 114.114.114.114 119.29.29.29"

[ -f $pidfile ] && kill -9 "$(cat $pidfile)" || echo "$$" > $pidfile

go_exit(){
	rm -f $pidfile
	exit 0
}

if [ "$(nvram get scutclient_watchcat)" != "1" ] || [ "$(nvram get scutclient_enable)" != "1" ]; then
	go_exit
fi

if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
#	logger -t "scutclient_watchcat" "WAN has no link!"
	go_exit
fi

for n in $net; do
	/bin/ping -c 3 "$n" -w 5 >/dev/null 2>&1
	if [ "$?" = "0" ]; then
		go_exit
	fi
done

if [ $(date '+%H') -lt 6 ]; then
	/usr/bin/scutclient.sh restart nolog > /dev/null 2>&1
else
	logger -t "scutclient_watchcat" "Connection failed, restart scutclient!"
	/usr/bin/scutclient.sh restart > /dev/null 2>&1
fi
go_exit
