#! /bin/sh
# $Id: iptables_display.sh,v 1.6 2016/02/09 09:37:44 nanard Exp $
IPTABLES=/sbin/iptables

#display all chains relative to miniupnpd
$IPTABLES -v -n -t nat -L PREROUTING
$IPTABLES -v -n -t nat -L MINIUPNPD
$IPTABLES -v -n -t nat -L POSTROUTING
$IPTABLES -v -n -t nat -L MINIUPNPD-POSTROUTING
$IPTABLES -v -n -t mangle -L PREROUTING
$IPTABLES -v -n -t mangle -L MINIUPNPD
$IPTABLES -v -n -t filter -L FORWARD
$IPTABLES -v -n -t filter -L MINIUPNPD

