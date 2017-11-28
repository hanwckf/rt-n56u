#!/bin/sh

set -e -o pipefail

loger(){
	logger -st "chnroute" "$1"
}

if [ "$(nvram get ss_update_chnroute)" != "1" ]; then
	exit 0
fi

if [ "$(mtk_esw 11)" = "WAN ports link state: 0" ]; then
	loger "WAN has no link!"
	exit 1
fi

wget -O- 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest' | \
    awk -F\| '/CN\|ipv4/ { printf("%s/%d\n", $4, 32-log($5)/log(2)) }' > \
    /tmp/chinadns_chnroute.txt

mv -f /tmp/chinadns_chnroute.txt /etc/storage/chinadns/chnroute.txt

mtd_storage.sh save > /dev/null 2>&1

if [ -f /usr/bin/chinadns.sh ]; then
	/usr/bin/chinadns.sh restart >/dev/null 2>&1
fi

if [ -f /usr/bin/shadowsocks.sh ]; then
	/usr/bin/shadowsocks.sh restart >/dev/null 2>&1
fi

loger "Update done"
