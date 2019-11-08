#!/bin/sh
pidfile="/var/scutclient_watchcat.pid"
net="114.114.114.114 119.29.29.29"
auth_flag="/tmp/scutclient_status"

[ -f $pidfile ] && kill -9 "$(cat $pidfile)" || echo "$$" > $pidfile

go_exit(){
	rm -f $pidfile
	exit 0
}

auth(){
	if [ $(date '+%H') -lt 6 ]; then
		# if NTP failed or force re-auth (default), then restart scutclient
		if [ $(date '+%Y') -lt 2019 ] || [ "$(nvram get scutclient_wdg_force)" != "0" ]; then
			/usr/bin/scutclient.sh restart nolog > /dev/null 2>&1
		fi
	else
		logger -t "scutclient_watchcat" "Connection failed ($1), restart scutclient!"
		/usr/bin/scutclient.sh restart > /dev/null 2>&1
	fi
	go_exit
}

if [ "$(nvram get scutclient_watchcat)" = "1" ] && \
  [ "$(nvram get scutclient_enable)" = "1" ] && \
  [ "$(mtk_esw 11)" != "WAN ports link state: 0" ]; then

	if [ "$(nvram get scutclient_wdg_flag)" = "1" ]; then
		if [ -f $auth_flag ] && [ "$(cat $auth_flag)" = "1" ]; then
			auth "flag"
		fi
	fi

	for n in $net; do
		/bin/ping -c 3 "$n" -w 5 >/dev/null 2>&1
		if [ "$?" = "0" ]; then
			go_exit
		fi
	done
	auth "ping"
else
	go_exit
fi
