#! /bin/sh
# $Id: iptables_flush.sh,v 1.6 2017/04/21 11:16:09 nanard Exp $
IPTABLES=/sbin/iptables

#flush all rules owned by miniupnpd
$IPTABLES -t nat -F MINIUPNPD
$IPTABLES -t nat -F MINIUPNPD-POSTROUTING
$IPTABLES -t filter -F MINIUPNPD
$IPTABLES -t mangle -F MINIUPNPD

