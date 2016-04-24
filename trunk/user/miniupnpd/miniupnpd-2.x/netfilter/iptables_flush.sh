#! /bin/sh
# $Id: iptables_flush.sh,v 1.5 2016/02/09 09:37:44 nanard Exp $
IPTABLES=/sbin/iptables

#flush all rules owned by miniupnpd
$IPTABLES -t nat -F MINIUPNPD
$IPTABLES -t nat -F MINIUPNPD-POSTROUTING
$IPTABLES -t filter -F MINIUPNPD
$IPTABLES -t mangle -F MINIUPNPD

