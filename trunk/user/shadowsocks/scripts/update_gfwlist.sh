#!/bin/sh

set -e -o pipefail

[ "$1" != "force" ] && [ "$(nvram get ss_update_gfwlist)" != "1" ] && exit 0
GFWLIST_URL="$(nvram get gfwlist_url)"

logger -st "gfwlist" "Starting update..."

rm -f /tmp/dnsmasq_gfwlist.conf
curl -k -s -o /tmp/dnsmasq_gfwlist.conf --connect-timeout 5 --retry 3 ${GFWLIST_URL:-"https://cokebar.github.io/gfwlist2dnsmasq/dnsmasq_gfwlist.conf"}

mkdir -p /etc/storage/gfwlist/
mv -f /tmp/dnsmasq_gfwlist.conf /etc/storage/gfwlist/dnsmasq_gfwlist.conf

mtd_storage.sh save >/dev/null 2>&1

restart_dhcpd
restart_dns

logger -st "gfwlist" "Update done"
