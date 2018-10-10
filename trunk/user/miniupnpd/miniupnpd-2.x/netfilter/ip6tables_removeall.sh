#! /bin/sh
# $Id: ip6tables_removeall.sh,v 1.2 2018/04/06 09:21:11 nanard Exp $

IPV6=1
EXT=1
. $(dirname "$0")/miniupnpd_functions.sh

#removing the MINIUPNPD chain for filter
if [ "$FDIRTY" = "${CHAIN}Chain" ]; then
	$IPTABLES -t filter -F $CHAIN
	$IPTABLES -t filter -D FORWARD -i $EXTIF ! -o $EXTIF -j $CHAIN
	$IPTABLES -t filter -X $CHAIN
elif [ "$FDIRTY" = "Chain" ]; then
	$IPTABLES -t filter -F $CHAIN
	$IPTABLES -t filter -X $CHAIN
fi
