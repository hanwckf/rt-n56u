#! /bin/sh
# $Id: iptables_flush.sh,v 1.7 2018/04/06 10:17:09 nanard Exp $

. $(dirname "$0")/miniupnpd_functions.sh

#flush all rules owned by miniupnpd
$IPTABLES -t nat -F $CHAIN
$IPTABLES -t nat -F $CHAIN-POSTROUTING
$IPTABLES -t filter -F $CHAIN
$IPTABLES -t mangle -F $CHAIN
