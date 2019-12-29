#!/bin/sh

set -e -o pipefail

[ "$1" != "force" ] && [ "$(nvram get ss_update_gfwlist)" != "1" ] && exit 0
#GFWLIST_URL="$(nvram get gfwlist_url)"

logger -st "gfwlist" "Starting update..."

rm -f /tmp/gfwlist_list.conf
rm -f /etc/storage/gfwlist/gfwlist_list.conf
curl -k -s -o /tmp/gfwlist_list.conf --connect-timeout 15 --retry 5 https://cokebar.github.io/gfwlist2dnsmasq/gfwlist_domain.txt

mkdir -p /etc/storage/gfwlist/
#logger -st "gfwlist" "Update......"
if [ $(nvram get pdnsd_enable) = 0 ]; then
awk '{printf("server=/%s/127.0.0.1#5353\nipset=/%s/gfwlist\n", $1, $1 )}' /tmp/gfwlist_list.conf > /etc/storage/gfwlist/gfwlist_list.conf
else
awk '{printf("ipset=/%s/gfwlist\n", $1, $1 )}' /tmp/gfwlist_list.conf > /etc/storage/gfwlist/gfwlist_list.conf
fi
mtd_storage.sh save >/dev/null 2>&1

/sbin/restart_dhcpd

logger -st "gfwlist" "Update done"
