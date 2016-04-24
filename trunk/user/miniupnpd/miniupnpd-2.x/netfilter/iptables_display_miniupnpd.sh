#! /bin/sh
# $Id: iptables_display_miniupnpd.sh,v 1.1 2016/02/12 15:23:29 nanard Exp $
IPTABLES=/sbin/iptables

#display miniupnpd chains
$IPTABLES -v -n -t nat -L MINIUPNPD
$IPTABLES -v -n -t nat -L MINIUPNPD-POSTROUTING
$IPTABLES -v -n -t mangle -L MINIUPNPD
$IPTABLES -v -n -t filter -L MINIUPNPD

