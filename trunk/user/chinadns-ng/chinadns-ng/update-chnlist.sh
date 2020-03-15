#!/bin/bash
url='https://raw.github.com/felixonmars/dnsmasq-china-list/master/accelerated-domains.china.conf'
data=$(curl -4sSkL "$url") || { echo "download failed, exit-code: $?"; exit 1; }
echo "$data" | awk -F/ '{print $2}' | sort | uniq >chnlist.txt
