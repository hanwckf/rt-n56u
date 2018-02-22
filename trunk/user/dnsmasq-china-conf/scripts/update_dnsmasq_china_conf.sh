#!/bin/sh
set -e -o pipefail
[ "$1" != "force" ] && [ "$(nvram get dnsmasq_china_conf_update)" != "1" ] && exit 1

url="https://coding.net/u/felixonmars/p/dnsmasq-china-list/git/raw/master"
acc_china_file="accelerated-domains.china.conf"
acc_apple_file="apple.china.conf"
dir="/tmp"
rm -f $dir/$acc_china_file
rm -f $dir/$acc_apple_file

if [ -f /usr/bin/openssl ]; then
	wget -q -T 5 -O $dir/$acc_china_file $url/$acc_china_file
	wget -q -T 5 -O $dir/$acc_apple_file $url/$acc_apple_file
else
	curl -k -s -o $dir/$acc_china_file --connect-timeout 5 --retry 3 $url/$acc_china_file
	curl -k -s -o $dir/$acc_apple_file --connect-timeout 5 --retry 3 $url/$acc_apple_file
fi

cat $dir/$acc_apple_file >> $dir/$acc_china_file
rm -f $dir/$acc_apple_file
[ ! -d /etc/storage/dnsmasq-china-conf ] && mkdir /etc/storage/dnsmasq-china-conf
mv -f $dir/$acc_china_file /etc/storage/dnsmasq-china-conf/$acc_china_file
mtd_storage.sh save >/dev/null 2>&1
restart_dns

logger -st "dnsmasq-china-conf" "Update done"
