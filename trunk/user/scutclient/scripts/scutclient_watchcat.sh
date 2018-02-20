#!/bin/sh
export PATH='/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin'
net_ip="202.38.193.188"

if [ "$(nvram get scutclient_watchcat)" != "1" ] || [ "$(nvram get scutclient_enable)" != "1" ]; then
	exit 0
fi

if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
	logger -st "scutclient_watchcat" "WAN has no link!"
	exit 1
fi

tries=0
while [ $tries -lt 3 ]
do
    if /bin/ping -c 1 $net_ip -W 1 >/dev/null
    then
        exit 0
    fi
    tries=$((tries+1))
done
logger -st "scutclient_watchcat" "Connection failed, restart scutclient!"
/bin/scutclient.sh restart
