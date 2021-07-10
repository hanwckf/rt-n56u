#!/bin/bash
set -o errexit
set -o pipefail
echo "create chnroute6 hash:net family inet6" >chnroute6.ipset
curl -4sSkL 'http://ftp.apnic.net/apnic/stats/apnic/delegated-apnic-latest' | grep CN | grep ipv6 | awk -F'|' '{printf("add chnroute6 %s/%d\n", $4, $5)}' >>chnroute6.ipset
