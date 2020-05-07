#!/bin/sh
# Copyright (C) 2018 Nick Peng (pymumu@gmail.com)
# Copyright (C) 2019 chongshengB
SMARTDNS_CONF_DIR="/etc/storage"
SMARTDNS_CONF="$SMARTDNS_CONF_DIR/smartdns.conf"
ADDRESS_CONF="$SMARTDNS_CONF_DIR/smartdns_address.conf"
BLACKLIST_IP_CONF="$SMARTDNS_CONF_DIR/smartdns_blacklist-ip.conf"
WHITELIST_IP_CONF="$SMARTDNS_CONF_DIR/smartdns_whitelist-ip.conf"
CUSTOM_CONF="$SMARTDNS_CONF_DIR/smartdns_custom.conf"
smartdns_file="/usr/bin/smartdns"
sdns_enable=`nvram get sdns_enable`
snds_name=`nvram get snds_name`
sdns_port=`nvram get sdns_port`
sdns_tcp_server=`nvram get sdns_tcp_server`
sdns_ipv6_server=`nvram get sdns_ipv6_server`
snds_ip_change=`nvram get snds_ip_change`
snds_ipv6=`nvram get snds_ipv6`
sdns_www=`nvram get sdns_www`
sdns_exp=`nvram get sdns_exp`
snds_redirect=`nvram get snds_redirect`
snds_cache=`nvram get snds_cache`
sdns_ttl=`nvram get sdns_ttl`
sdns_ttl_min=`nvram get sdns_ttl_min`
sdns_ttl_max=`nvram get sdns_ttl_max`
sdnse_enable=`nvram get sdnse_enable`
sdnse_port=`nvram get sdnse_port`
sdnse_tcp=`nvram get sdnse_tcp`
sdnse_speed=`nvram get sdnse_speed`
sdnse_name=`nvram get sdnse_name`
sdnse_address=`nvram get sdnse_address`
sdnse_ns=`nvram get sdnse_ns`
sdnse_ipset=`nvram get sdnse_ipset`
sdnse_as=`nvram get sdnse_as`
sdnse_ipc=`nvram get sdnse_ipc`
sdnse_cache=`nvram get sdnse_cache`
ss_white=`nvram get ss_white`
ss_black=`nvram get ss_black`

check_ss(){
if [ $(nvram get ss_enable) = 1 ] && [ $(nvram get ss_run_mode) = "router" ] && [ $(nvram get pdnsd_enable) = 0 ]; then
logger -t "SmartDNS" "系统检测到SS模式为绕过大陆模式，并且启用了pdnsd,请先调整SS解析使用SmartDNS+手动配置模式！程序将退出。"
nvram set sdns_enable=0
exit 0
fi
}

get_tz()
{
	SET_TZ=""
	for tzfile in /etc/TZ
	do
		if [ ! -e "$tzfile" ]; then
			continue
		fi		
		tz="`cat $tzfile 2>/dev/null`"
	done	
	if [ -z "$tz" ]; then
		return	
	fi	
	SET_TZ=$tz
}
gensmartconf(){
rm -f $SMARTDNS_CONF
touch $SMARTDNS_CONF
echo "server-name $snds_name" >> $SMARTDNS_CONF
	if [ "$sdns_ipv6_server" = "1" ]; then
		echo "bind" "[::]:$sdns_port" >> $SMARTDNS_CONF
	else
		echo "bind" ":$sdns_port" >> $SMARTDNS_CONF
	fi
	if [ "$sdns_tcp_server" = "1" ]; then
		if [ "$sdns_ipv6_server" = "1" ]; then
			echo "bind-tcp" "[::]:$sdns_port" >> $SMARTDNS_CONF
		else
			echo "bind-tcp" ":$sdns_port" >> $SMARTDNS_CONF
		fi
	fi
gensdnssecond
echo "cache-size $snds_cache" >> $SMARTDNS_CONF
if [ $snds_ip_change -eq 1 ];then
echo "dualstack-ip-selection yes" >> $SMARTDNS_CONF
echo "dualstack-ip-selection-threshold $(nvram get snds_ip_change_time)" >> $SMARTDNS_CONF
elif [ $snds_ipv6 -eq 1 ];then
echo "force-AAAA-SOA yes" >> $SMARTDNS_CONF
fi
if [ $sdns_www -eq 1 ];then
echo "prefetch-domain yes" >> $SMARTDNS_CONF
else
echo "prefetch-domain no" >> $SMARTDNS_CONF
fi
if [ $sdns_exp -eq 1 ];then
echo "serve-expired yes" >> $SMARTDNS_CONF
else
echo "serve-expired no" >> $SMARTDNS_CONF
fi
echo "log-level info" >> $SMARTDNS_CONF
listnum=`nvram get sdnss_staticnum_x`
for i in $(seq 1 $listnum)
do
j=`expr $i - 1`
sdnss_enable=`nvram get sdnss_enable_x$j`
if  [ $sdnss_enable -eq 1 ]; then
sdnss_name=`nvram get sdnss_name_x$j`
sdnss_ip=`nvram get sdnss_ip_x$j`
sdnss_port=`nvram get sdnss_port_x$j`
sdnss_type=`nvram get sdnss_type_x$j`
sdnss_ipc=`nvram get sdnss_ipc_x$j`
sdnss_named=`nvram get sdnss_named_x$j`
sdnss_non=`nvram get sdnss_non_x$j`
sdnss_ipset=`nvram get sdnss_ipset_x$j`
ipc=""
named=""
non=""
sipset=""
if [ $sdnss_ipc = "whitelist" ]; then
ipc="-whitelist-ip"
elif [ $sdnss_ipc = "blacklist" ]; then
ipc="-blacklist-ip"
fi
if [ $sdnss_named != "" ]; then
named="-group $sdnss_named"
fi
if [ $sdnss_non = "1" ]; then
non="-exclude-default-group"
fi
if [ $sdnss_type = "tcp" ]; then
if [ $sdnss_port = "default" ]; then
echo "server-tcp $sdnss_ip:53 $ipc $named $non" >> $SMARTDNS_CONF
else
echo "server-tcp $sdnss_ip:$sdnss_port $ipc $named $non" >> $SMARTDNS_CONF
fi
elif [ $sdnss_type = "udp" ]; then
if [ $sdnss_port = "default" ]; then
echo "server $sdnss_ip:53 $ipc $named $non" >> $SMARTDNS_CONF
else
echo "server $sdnss_ip:$sdnss_port $ipc $named $non" >> $SMARTDNS_CONF
fi
elif [ $sdnss_type = "tls" ]; then
if [ $sdnss_port = "default" ]; then
echo "server-tls $sdnss_ip:53 $ipc $named $non" >> $SMARTDNS_CONF
else
echo "server-tls $sdnss_ip:$sdnss_port $ipc $named $non" >> $SMARTDNS_CONF
fi
elif [ $sdnss_type = "https" ]; then
if [ $sdnss_port = "default" ]; then
echo "server-https $sdnss_ip $ipc $named $non" >> $SMARTDNS_CONF
fi	
fi
if [ $sdnss_ipset != "" ]; then
#ipset add gfwlist $sdnss_ipset 2>/dev/null
CheckIPAddr $sdnss_ipset
if [ "$?" == "1" ];then
echo "ipset /$sdnss_ipset/smartdns" >> $SMARTDNS_CONF
else
ipset add smartdns $sdnss_ipset 2>/dev/null
fi
fi	
fi
done
if [ $ss_white = "1" ]; then
rm -f /tmp/whitelist.conf
logger -t "SmartDNS" "开始处理白名单IP"
awk '{printf("whitelist-ip %s\n", $1, $1 )}' /etc/storage/chinadns/chnroute.txt >> /tmp/whitelist.conf
echo "conf-file /tmp/whitelist.conf" >> $SMARTDNS_CONF
fi
if [ $ss_black = "1" ]; then
rm -f /tmp/blacklist.conf
logger -t "SmartDNS" "开始处理黑名单IP"
awk '{printf("blacklist-ip %s\n", $1, $1 )}' /etc/storage/chinadns/chnroute.txt >> /tmp/blacklist.conf
echo "conf-file /tmp/blacklist.conf" >> $SMARTDNS_CONF
fi
}

gensdnssecond(){
if  [ $sdnse_enable -eq 1 ]; then
ARGS=""
ADDR=""
if [ "$sdnse_speed" = "1" ]; then
	ARGS="$ARGS -no-speed-check"
fi
if [ ! -z "$sdnse_name" ]; then
		ARGS="$ARGS -group $sdnse_name"
	fi
if [ "$sdnse_address" = "1" ]; then
		ARGS="$ARGS -no-rule-addr"
	fi
	if [ "$sdnse_ns" = "1" ]; then
		ARGS="$ARGS -no-rule-nameserver"
	fi
	if [ "$sdnse_ipset" = "1" ]; then
		ARGS="$ARGS -no-rule-ipset"
	fi
	if [ "$sdnse_as" = "1" ]; then
		ARGS="$ARGS -no-rule-soa"
	fi
	if [ "$sdnse_ipc" = "1" ]; then
		ARGS="$ARGS -no-dualstack-selection"
	fi
	if [ "$sdnse_cache" = "1" ]; then
		ARGS="$ARGS -no-cache"
	fi
	if [ "$sdns_ipv6_server" = "1" ]; then
		ADDR="[::]"
	else
		ADDR=""
	fi
echo "bind" "$ADDR:$sdnse_port $ARGS" >> $SMARTDNS_CONF
	if [ "$sdnse_tcp" = "1" ]; then
		echo "bind-tcp" "$ADDR:$sdnse_port$ARGS" >> $SMARTDNS_CONF
	fi
fi
}

change_dns() {
sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
no-resolv
server=127.0.0.1#$sdns_port
EOF
/sbin/restart_dhcpd
logger -t "SmartDNS" "添加DNS转发到$sdns_port端口"
}
del_dns() {
sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
/sbin/restart_dhcpd
}

set_iptable()
{
	ipv6_server=$1
	tcp_server=$2

	IPS="`ifconfig | grep "inet addr" | grep -v ":127" | grep "Bcast" | awk '{print $2}' | awk -F : '{print $2}'`"
	for IP in $IPS
	do
		if [ "$tcp_server" == "1" ]; then
			iptables -t nat -A PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports $sdns_port >/dev/null 2>&1
		fi
		iptables -t nat -A PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports $sdns_port >/dev/null 2>&1
	done

	if [ "$ipv6_server" == 0 ]; then
		return
	fi

	IPS="`ifconfig | grep "inet6 addr" | grep -v " fe80::" | grep -v " ::1" | grep "Global" | awk '{print $3}'`"
	for IP in $IPS
	do
		if [ "$tcp_server" == "1" ]; then
			ip6tables -t nat -A PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports $sdns_port >/dev/null 2>&1
		fi
		ip6tables -t nat -A PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports $sdns_port >/dev/null 2>&1
	done
logger -t "SmartDNS" "重定向53端口"
}

clear_iptable()
{
	local OLD_PORT="$1"
	local ipv6_server=$2
	IPS="`ifconfig | grep "inet addr" | grep -v ":127" | grep "Bcast" | awk '{print $2}' | awk -F : '{print $2}'`"
	for IP in $IPS
	do
		iptables -t nat -D PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
		iptables -t nat -D PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
	done

	if [ "$ipv6_server" == 0 ]; then
		return
	fi

	IPS="`ifconfig | grep "inet6 addr" | grep -v " fe80::" | grep -v " ::1" | grep "Global" | awk '{print $3}'`"
	for IP in $IPS
	do
		ip6tables -t nat -D PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
		ip6tables -t nat -D PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
	done
	
}

start_smartdns(){
rm -f /tmp/sdnsipset.conf
args=""
logger -t "SmartDNS" "创建配置文件."
ipset -N smartdns hash:net 2>/dev/null
gensmartconf

grep -v '^#' $ADDRESS_CONF | grep -v "^$" >> $SMARTDNS_CONF
grep -v '^#' $BLACKLIST_IP_CONF | grep -v "^$" >> $SMARTDNS_CONF
grep -v '^#' $WHITELIST_IP_CONF | grep -v "^$" >> $SMARTDNS_CONF
grep -v '^#' $CUSTOM_CONF | grep -v "^$" >> $SMARTDNS_CONF
#grep -v ^! /tmp/whitelist.txt >> $SMARTDNS_CONF
#rm -f /tmp/whitelist.txt
#grep -v ^! /tmp/blacklist.txt >> $SMARTDNS_CONF
#rm -f /tmp/blacklist.txt
if [ "$sdns_coredump" = "1" ]; then
		args="$args -S"
	fi
	#get_tz
	#if [ ! -z "$SET_TZ" ]; then
#		procd_set_param env TZ="$SET_TZ"
	#fi
$smartdns_file -f -c $SMARTDNS_CONF $args &>/dev/null &
logger -t "SmartDNS" "SmartDNS启动成功"
if [ $snds_redirect = "2" ]; then
		set_iptable $sdns_ipv6_server $sdns_tcp_server
	elif [ $snds_redirect = "1" ]; then
		change_dns
	fi

}

CheckIPAddr()
{
echo $1|grep "^[0-9]\{1,3\}\.\([0-9]\{1,3\}\.\)\{2\}[0-9]\{1,3\}$" > /dev/null;
#IP地址必须为全数字
        if [ $? -ne 0 ]
        then
                return 1
        fi
        ipaddr=$1
        a=`echo $ipaddr|awk -F . '{print $1}'`  #以"."分隔，取出每个列的值
        b=`echo $ipaddr|awk -F . '{print $2}'`
        c=`echo $ipaddr|awk -F . '{print $3}'`
        d=`echo $ipaddr|awk -F . '{print $4}'`
        for num in $a $b $c $d
        do
                if [ $num -gt 255 ] || [ $num -lt 0 ]    #每个数值必须在0-255之间
                then
                        return 1
                fi
        done
                return 0
}

stop_smartdns(){
rm -f /tmp/whitelist.conf
rm -f /tmp/blacklist.conf
smartdns_process=`pidof smartdns`
if [ -n "$smartdns_process" ];then 
	logger -t "SmartDNS" "关闭smartdns进程..."
	killall smartdns >/dev/null 2>&1
	kill -9 "$smartdns_process" >/dev/null 2>&1
fi
ipset -X smartdns 2>/dev/null
del_dns
clear_iptable $sdns_port $sdns_ipv6_server
if [ "$snds_redirect" = "2" ]; then
		clear_iptable $sdns_port $sdns_ipv6_server
	elif [ "$snds_redirect" = "1" ]; then
		del_dns
	fi
logger -t "SmartDNS" "SmartDNS已关闭"
}

case $1 in
start)
    check_ss
	start_smartdns
	;;
stop)
	stop_smartdns
	;;
*)
	echo "check"
	;;
esac
