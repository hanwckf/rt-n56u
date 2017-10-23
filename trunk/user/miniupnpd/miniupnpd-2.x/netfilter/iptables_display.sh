#! /bin/sh
# $Id: iptables_display.sh,v 1.7 2017/04/21 11:16:09 nanard Exp $
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

