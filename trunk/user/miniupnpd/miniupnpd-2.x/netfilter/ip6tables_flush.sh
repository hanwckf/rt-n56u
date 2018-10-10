#! /bin/sh
# $Id: ip6tables_flush.sh,v 1.2 2018/04/06 09:21:11 nanard Exp $

IPV6=1
. $(dirname "$0")/miniupnpd_functions.sh

#flush all rules owned by miniupnpd
$IPTABLES -t filter -F $CHAIN
