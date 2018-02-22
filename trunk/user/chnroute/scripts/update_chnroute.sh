#!/bin/sh

set -e -o pipefail

[ "$1" != "force" ] && [ "$(nvram get ss_update_chnroute)" != "1" ] && exit 0

rm -f /tmp/chinadns_chnroute.txt
wget -O- 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest' | \
    awk -F\| '/CN\|ipv4/ { printf("%s/%d\n", $4, 32-log($5)/log(2)) }' > \
    /tmp/chinadns_chnroute.txt

[ ! -d /etc/storage/chinadns/ ] && mkdir /etc/storage/chinadns/
mv -f /tmp/chinadns_chnroute.txt /etc/storage/chinadns/chnroute.txt

mtd_storage.sh save >/dev/null 2>&1

[ -f /usr/bin/chinadns.sh ] && /usr/bin/chinadns.sh restart >/dev/null 2>&1
[ -f /usr/bin/shadowsocks.sh ] && /usr/bin/shadowsocks.sh restart >/dev/null 2>&1

logger -st "chnroute" "Update done"
