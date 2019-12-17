#!/bin/sh

set -e -o pipefail

[ "$1" != "force" ] && [ "$(nvram get ss_update_gfwlist)" != "1" ] && exit 0
GFWLIST_URL="$(nvram get gfwlist_url)"

logger -st "gfwlist" "开始更新gfwlist列表"

rm -f /tmp/gfwlist_list.conf
curl -k -s -o /tmp/gfwlist_list.conf --connect-timeout 5 --retry 3 ${GFWLIST_URL:-"https://cokebar.github.io/gfwlist2dnsmasq/gfwlist_domain.txt"}

mkdir -p /etc/storage/gfwlist/
mv -f /tmp/gfwlist_list.conf /etc/storage/gfwlist/gfwlist_list.conf

mtd_storage.sh save >/dev/null 2>&1

/sbin/restart_dhcpd

logger -st "gfwlist" "gfwlist更新成功"
