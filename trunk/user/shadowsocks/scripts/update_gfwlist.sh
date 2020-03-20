#!/bin/sh

set -e -o pipefail
[ "$1" != "force" ] && [ "$(nvram get ss_update_gfwlist)" != "1" ] && exit 0
#GFWLIST_URL="$(nvram get gfwlist_url)"
logger -st "gfwlist" "Starting update..."
curl -k -s -o /tmp/gfwlist_list.conf --connect-timeout 15 --retry 5 https://cokebar.github.io/gfwlist2dnsmasq/gfwlist_domain.txt
count=`awk '{print NR}' /tmp/gfwlist_list.conf|tail -n1`
if [ $count -gt 1000 ]; then
rm -f /etc/storage/gfwlist/gfwlist_list.conf
cp -r /tmp/gfwlist_list.conf /etc/storage/gfwlist/gfwlist_list.conf
mtd_storage.sh save >/dev/null 2>&1
mkdir -p /etc/storage/gfwlist/
logger -st "gfwlist" "Update done"
if [ $(nvram get ss_enable) = 1 ]; then
logger -st "SS" "重启ShadowSocksR Plus+..."
/usr/bin/shadowsocks.sh stop
/usr/bin/shadowsocks.sh start
fi
else
logger -st "gfwlist" "列表下载失败,请重试！"
fi
rm -f /tmp/gfwlist_list.conf