#! /bin/sh
# $Id: iptables_display_miniupnpd.sh,v 1.2 2018/04/06 09:21:11 nanard Exp $

. $(dirname "$0")/miniupnpd_functions.sh

#display miniupnpd chains
$IPTABLES -v -n -t nat -L $CHAIN
$IPTABLES -v -n -t nat -L $CHAIN-POSTROUTING
$IPTABLES -v -n -t mangle -L $CHAIN
$IPTABLES -v -n -t filter -L $CHAIN
