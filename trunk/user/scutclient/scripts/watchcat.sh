#!/bin/sh
export PATH='/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin'
if [ "$(nvram get scutclient_enable)" != "1" ]; then
	exit 0
fi

if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
	echo "WAN has no link!"
	logger -t "scutclient_watchcat" "WAN has no link!"
	exit 1
fi

tries=0
while [[ $tries -lt 3 ]]
do
    if /bin/ping -c 1 202.38.193.188 -W 1 >/dev/null
    then
        exit 0
    fi
    tries=$((tries+1))
done
echo "Connection failed, restart scutclient!"
logger -t "scutclient_watchcat" "Connection failed, restart scutclient!"
scutclient.sh restart