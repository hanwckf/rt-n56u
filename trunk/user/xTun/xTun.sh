#!/bin/sh

IFACE=$(nvram get xTun_iface)
CIDR=$(nvram get xTun_cidr)
SERVER=$(nvram get xTun_server)
PORT=$(nvram get xTun_port)
TCP=$(nvram get xTun_tcp)
KEY=$(nvram get xTun_key)

DNS=$(nvram get xTun_dns)
BLACK_LIST=$(nvram get xTun_black_list)

IP_ROUTE_TABLE=xTun
IP_ROUTE_TABLE_ID=200
FWMARK="0x023/0x023"
SETNAME=wall
CHAIN=xTun

get_arg_tcp() {
	if [ "$TCP" = "1" ]; then
		echo "-t"
	fi
}

start() {
    xTun -i $IFACE -I $CIDR -k $KEY -c $SERVER -p $PORT $(get_arg_tcp)
    net_start
    acl add
}

stop() {
    net_stop
    acl del
    xTun --signal stop
}

shutdown() {
    net_stop
    acl del
    xTun --signal quit
}

restart() {
    stop
    start
}

net_start() {
    sysctl -w net.ipv4.ip_forward=1 >/dev/null

    for f in /proc/sys/net/ipv4/conf/*/rp_filter; do
        echo 0 > $f
    done

    iptables -t nat -N $CHAIN >/dev/null 2>&1 || (
        iptables -t nat -D POSTROUTING -j $CHAIN
        iptables -t nat -F $CHAIN
        iptables -t nat -Z $CHAIN
    )
    iptables -t nat -A $CHAIN -o $IFACE -j MASQUERADE
    iptables -t nat -A POSTROUTING -j $CHAIN

    iptables -N $CHAIN >/dev/null 2>&1 || (
        iptables -D FORWARD -j $CHAIN
        iptables -F $CHAIN
        iptables -Z $CHAIN
    )
    iptables -I $CHAIN 1 -i $IFACE -m state --state RELATED,ESTABLISHED -j ACCEPT
    iptables -I $CHAIN 1 -o $IFACE -j ACCEPT
    iptables -I FORWARD -j $CHAIN

    iptables -t mangle -N $CHAIN >/dev/null 2>&1 || (
        iptables -t mangle -D PREROUTING -j $CHAIN
        iptables -t mangle -D OUTPUT -j $CHAIN
        iptables -t mangle -F $CHAIN
        iptables -t mangle -Z $CHAIN
    )
    ipset -N $SETNAME iphash -exist
    iptables -t mangle -A $CHAIN -m set --match-set $SETNAME dst -j MARK --set-mark $FWMARK
    iptables -t mangle -A PREROUTING -j $CHAIN
    iptables -t mangle -A OUTPUT -j $CHAIN

    xTun_rule_ids=`ip rule list | grep "lookup $IP_ROUTE_TABLE" | sed 's/://g' | awk '{print $1}'`
    for rule_id in $xTun_rule_ids
    do
        ip rule del prio $rule_id
    done

    # grep -q $IP_ROUTE_TABLE /etc/iproute2/rt_tables
    # if [ "$?" -ne 0 ]; then
    #     echo "$IP_ROUTE_TABLE_ID $IP_ROUTE_TABLE" >> /etc/iproute2/rt_tables
    # fi

    ip route add default dev $IFACE table $IP_ROUTE_TABLE_ID
    ip route list | grep -q "$DNS dev $IFACE" || ip route add $DNS dev $IFACE
    ip rule list | grep -q "fwmark $FWMARK lookup $IP_ROUTE_TABLE_ID" || ip rule add fwmark $FWMARK table $IP_ROUTE_TABLE_ID

    ip route flush cache
}

net_stop() {
    iptables -t nat -D POSTROUTING -j $CHAIN 2>/dev/null
    iptables -t nat -F $CHAIN 2>/dev/null
    iptables -t nat -X $CHAIN 2>/dev/null

    iptables -D FORWARD -j $CHAIN 2>/dev/null
    iptables -F $CHAIN 2>/dev/null
    iptables -X $CHAIN 2>/dev/null

    iptables -t mangle -D PREROUTING -j $CHAIN 2>/dev/null
    iptables -t mangle -D OUTPUT -j $CHAIN 2>/dev/null
    iptables -t mangle -F $CHAIN 2>/dev/null
    iptables -t mangle -X $CHAIN 2>/dev/null

    ip route del default dev $IFACE table $IP_ROUTE_TABLE_ID >/dev/null 2>&1
    ip route del $DNS dev $IFACE >/dev/null 2>&1
    xTun_rule_ids=`ip rule list | grep "lookup $IP_ROUTE_TABLE_ID" | sed 's/://g' | awk '{print $1}'`
    for rule_id in $xTun_rule_ids
    do
        ip rule del prio $rule_id
    done

    ip route flush cache
}

acl() {
    if [ ! -f $BLACK_LIST ]; then
        return
    fi

    while read line;do
        [ -z "$line" ] && continue
        case "$line" in \#*) continue ;; esac
        ipset $1 $SETNAME $line --exist
    done < $BLACK_LIST
}

show_help() {
    echo "Usage: $ProgName <command> [options]"
    echo "Commands:"
    echo "    start     start tun"
    echo "    stop      stop tun"
    echo "    restart   restart tun"
    echo ""
    echo "For help with each command run:"
    echo "$ProgName <command> -h|--help"
    echo ""
}

ProgName=$(basename $0)

command=$1
case $command in
    "" | "-h" | "--help")
        show_help
        ;;
    *)
        shift
        ${command} $@
        if [ $? = 127 ]; then
            echo "Error: '$command' is not a known command." >&2
            echo "       Run '$ProgName --help' for a list of known commands." >&2
            exit 1
        fi
        ;;
esac
