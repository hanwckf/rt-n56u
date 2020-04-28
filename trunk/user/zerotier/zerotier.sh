#!/bin/sh
#20200426 chongshengB
PROG=/usr/bin/zerotier-one
config_path="/etc/storage/zerotier-one"
start_instance() {
	cfg="$1"
	echo $cfg
	port=""
	args=""
	secret="$(nvram get zerotier_secret)"
	if [ ! -d "$config_path" ]; then
		mkdir -p $config_path
	fi
	mkdir -p $config_path/networks.d
	if [ -n "$port" ]; then
		args="$args -p$port"
	fi
	if [ -z "$secret" ]; then
		logger -t "zerotier" "设备密匙为空,正在生成密匙,请稍后..."
		sf="/tmp/zt.$cfg.secret"
		zerotier-idtool generate "$sf" >/dev/null
		[ $? -ne 0 ] && return 1
		secret="$(cat $sf)"
		rm "$sf"
		nvram set zerotier_secret="$secret"
		nvram commit
	fi
	if [ -n "$secret" ]; then
		logger -t "zerotier" "找到密匙,正在写入文件,请稍后..."
		echo "$secret" >$config_path/identity.secret
		rm -f $config_path/identity.public
	fi

	add_join $(nvram get zerotier_id)

	$PROG $args $config_path >/dev/null 2>&1 &

	rules
}

add_join() {
		touch $config_path/networks.d/$1.conf
}


rules() {
	while [ "$(ifconfig | grep zt | awk '{print $1}')" = "" ]; do
		sleep 1
	done
	nat_enable=$(nvram get zerotier_nat)
	zt0=$(ifconfig | grep zt | awk '{print $1}')
	logger -t "zerotier" "zt interface $zt0 is started!"
	del_rules
	iptables -A INPUT -i $zt0 -j ACCEPT
	iptables -A FORWARD -i $zt0 -o $zt0 -j ACCEPT
	iptables -A FORWARD -i $zt0 -j ACCEPT
	if [ $nat_enable -eq 1 ]; then
		iptables -t nat -A POSTROUTING -o $zt0 -j MASQUERADE
		while [ "$(ip route | grep "dev $zt0  proto" | awk '{print $1}')" = "" ]; do
		sleep 1
	    done
		ip_segment=`ip route | grep "dev $zt0  proto" | awk '{print $1}'`
		iptables -t nat -A POSTROUTING -s $ip_segment -j MASQUERADE
		zero_route "add"
	fi

}

del_rules() {
	zt0=$(ifconfig | grep zt | awk '{print $1}')
	ip_segment=`ip route | grep "dev $zt0  proto" | awk '{print $1}'`
	iptables -D FORWARD -i $zt0 -j ACCEPT 2>/dev/null
	iptables -D FORWARD -o $zt0 -j ACCEPT 2>/dev/null
	iptables -D FORWARD -i $zt0 -o $zt0 -j ACCEPT
	iptables -D INPUT -i $zt0 -j ACCEPT 2>/dev/null
	iptables -t nat -D POSTROUTING -o $zt0 -j MASQUERADE 2>/dev/null
	iptables -t nat -D POSTROUTING -s $ip_segment -j MASQUERADE 2>/dev/null
}

zero_route(){
	rulesnum=`nvram get zero_staticnum_x`
	for i in $(seq 1 $rulesnum)
	do
		j=`expr $i - 1`
		route_enable=`nvram get zero_enable_x$j`
		zero_ip=`nvram get zero_ip_x$j`
		zero_route=`nvram get zero_route_x$j`
		if [ "$1" = "add" ]; then
		if [ $route_enable -ne 0 ]; then
			ip route add $zero_ip via $zero_route dev $zt0
			echo "$zt0"
		fi
	else
		ip route del $zero_ip via $zero_route dev $zt0
	fi
	done
}

start_zero() {
	logger -t "zerotier" "正在启动zerotier"
	kill_z
	start_instance 'zerotier'

}
kill_z() {
	killall -9 zerotier-one
}
stop_zero() {
	logger -t "zerotier" "关闭zerotier"
	del_rules
	zero_route "del"
	kill_z
	rm -rf $config_path
}

case $1 in
start)
	start_zero
	;;
stop)
	stop_zero
	;;
*)
	echo "check"
	#exit 0
	;;
esac