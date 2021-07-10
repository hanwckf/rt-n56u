#!/bin/sh
#
# Copyright (C) 2017 openwrt-ssr
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
# Copyright (C) 2018 lean <coolsnowwolf@gmail.com>
# Copyright (C) 2019 chongshengB <bkye@vip.qq.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
NAME=shadowsocksr
http_username=`nvram get http_username`
CONFIG_FILE=/tmp/${NAME}.json
CONFIG_UDP_FILE=/tmp/${NAME}_u.json
CONFIG_SOCK5_FILE=/tmp/${NAME}_s.json
CONFIG_KUMASOCKS_FILE=/tmp/kumasocks.toml
v2_json_file="/tmp/v2-redir.json"
trojan_json_file="/tmp/tj-redir.json"
server_count=0
redir_tcp=0
v2ray_enable=0
redir_udp=0
tunnel_enable=0
local_enable=0
pdnsd_enable_flag=0
chinadnsng_enable_flag=0
wan_bp_ips="/tmp/whiteip.txt"
wan_fw_ips="/tmp/blackip.txt"
lan_fp_ips="/tmp/lan_ip.txt"
run_mode=`nvram get ss_run_mode`
ss_turn=`nvram get ss_turn`
lan_con=`nvram get lan_con`
GLOBAL_SERVER=`nvram get global_server`
socks=""

find_bin() {
	case "$1" in
	ss) ret="/usr/bin/ss-redir" ;;
	ss-local) ret="/usr/bin/ss-local" ;;
	ssr) ret="/usr/bin/ssr-redir" ;;
	ssr-local) ret="/usr/bin/ssr-local" ;;
	ssr-server) ret="/usr/bin/ssr-server" ;;
	v2ray) ret="/usr/bin/v2ray" ;;
	xray) ret="/usr/bin/v2ray" ;;
	trojan) ret="/usr/bin/trojan" ;;
	socks5) ret="/usr/bin/ipt2socks" ;;
	esac
	echo $ret
}

gen_config_file() {

	fastopen="false"
	case "$2" in
	0) config_file=$CONFIG_FILE && local stype=$(nvram get d_type) ;;
	1) config_file=$CONFIG_UDP_FILE && local stype=$(nvram get ud_type) ;;
	*) config_file=$CONFIG_SOCK5_FILE && local stype=$(nvram get s5_type) ;;
	esac
local type=$stype
	case "$type" in
	ss)
		lua /etc_ro/ss/genssconfig.lua $1 $3 >$config_file
		sed -i 's/\\//g' $config_file
		;;
	ssr)
		lua /etc_ro/ss/genssrconfig.lua $1 $3 >$config_file
		sed -i 's/\\//g' $config_file
		;;
	trojan)
		tj_bin="/usr/bin/trojan"
		if [ "$2" = "0" ]; then
		lua /etc_ro/ss/gentrojanconfig.lua $1 nat 1080 >$trojan_json_file
		sed -i 's/\\//g' $trojan_json_file
		else
		lua /etc_ro/ss/gentrojanconfig.lua $1 client 10801 >/tmp/trojan-ssr-reudp.json
		sed -i 's/\\//g' /tmp/trojan-ssr-reudp.json
		fi
		;;
	v2ray)
		v2_bin="/usr/bin/v2ray"
		v2ray_enable=1
		if [ "$2" = "1" ]; then
		lua /etc_ro/ss/genv2config.lua $1 udp 1080 >/tmp/v2-ssr-reudp.json
		sed -i 's/\\//g' /tmp/v2-ssr-reudp.json
		else
		lua /etc_ro/ss/genv2config.lua $1 tcp 1080 >$v2_json_file
		sed -i 's/\\//g' $v2_json_file
		fi
		;;
	xray)
		v2_bin="/usr/bin/v2ray"
		v2ray_enable=1
		if [ "$2" = "1" ]; then
		lua /etc_ro/ss/genxrayconfig.lua $1 udp 1080 >/tmp/v2-ssr-reudp.json
		sed -i 's/\\//g' /tmp/v2-ssr-reudp.json
		else
		lua /etc_ro/ss/genxrayconfig.lua $1 tcp 1080 >$v2_json_file
		sed -i 's/\\//g' $v2_json_file
		fi
		;;	
	esac
}

get_arg_out() {
	router_proxy="1"
	case "$router_proxy" in
	1) echo "-o" ;;
	2) echo "-O" ;;
	esac
}

start_rules() {
    logger -t "SS" "正在添加防火墙规则..."
	lua /etc_ro/ss/getconfig.lua $GLOBAL_SERVER > /tmp/server.txt
	server=`cat /tmp/server.txt` 
	cat /etc/storage/ss_ip.sh | grep -v '^!' | grep -v "^$" >$wan_fw_ips
	cat /etc/storage/ss_wan_ip.sh | grep -v '^!' | grep -v "^$" >$wan_bp_ips
	#resolve name
	if echo $server | grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$" >/dev/null; then
		server=${server}
	elif [ "$server" != "${server#*:[0-9a-fA-F]}" ]; then
		server=${server}
	else
		server=$(ping ${server} -s 1 -c 1 | grep PING | cut -d'(' -f 2 | cut -d')' -f1)
		if echo $server | grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$" >/dev/null; then
			echo $server >/etc/storage/ssr_ip
		else
			server=$(cat /etc/storage/ssr_ip)
		fi
	fi
	local_port="1080"
	lan_ac_ips=$lan_ac_ips
	lan_ac_mode="b"
	#if [ "$GLOBAL_SERVER" == "$UDP_RELAY_SERVER" ]; then
	#	ARG_UDP="-u"
	if [ "$UDP_RELAY_SERVER" != "nil" ]; then
		ARG_UDP="-U"
		lua /etc_ro/ss/getconfig.lua $UDP_RELAY_SERVER > /tmp/userver.txt
	    udp_server=`cat /tmp/userver.txt` 
		udp_local_port="1080"
	fi
	if [ -n "$lan_ac_ips" ]; then
		case "$lan_ac_mode" in
		w | W | b | B) ac_ips="$lan_ac_mode$lan_ac_ips" ;;
		esac
	fi
	#ac_ips="b"
	gfwmode=""
	if [ "$run_mode" = "gfw" ]; then
		gfwmode="-g"
	elif [ "$run_mode" = "router" ]; then
		gfwmode="-r"
	elif [ "$run_mode" = "oversea" ]; then
		gfwmode="-c"
	elif [ "$run_mode" = "all" ]; then
		gfwmode="-z"
	fi
	if [ "$lan_con" = "0" ]; then
		rm -f $lan_fp_ips
		lancon="all"
		lancons="全部IP走代理"
		cat /etc/storage/ss_lan_ip.sh | grep -v '^!' | grep -v "^$" >$lan_fp_ips
	elif [ "$lan_con" = "1" ]; then
		rm -f $lan_fp_ips
		lancon="bip"
		lancons="指定IP走代理,请到规则管理页面添加需要走代理的IP。"
		cat /etc/storage/ss_lan_bip.sh | grep -v '^!' | grep -v "^$" >$lan_fp_ips
	fi
	dports=$(nvram get s_dports)
	if [ $dports = "0" ]; then
		proxyport=" "
	else
		proxyport="-m multiport --dports 22,53,587,465,995,993,143,80,443"
	fi
	/usr/bin/ss-rules \
	-s "$server" \
	-l "$local_port" \
	-S "$udp_server" \
	-L "$udp_local_port" \
	-a "$ac_ips" \
	-i "" \
	-b "$wan_bp_ips" \
	-w "$wan_fw_ips" \
	-p "$lan_fp_ips" \
	-G "$lan_gm_ips" \
	-G "$lan_gm_ips" \
	-D "$proxyport" \
	-k "$lancon" \
	$(get_arg_out) $gfwmode $ARG_UDP
	return $?
}

start_redir_tcp() {
	ARG_OTA=""
	gen_config_file $GLOBAL_SERVER 0 1080
	stype=$(nvram get d_type)
	local bin=$(find_bin $stype)
	[ ! -f "$bin" ] && echo "$(date "+%Y-%m-%d %H:%M:%S") Main node:Can't find $bin program, can't start!" >>/tmp/ssrplus.log && return 1
	if [ "$(nvram get ss_threads)" = "0" ]; then
		threads=$(cat /proc/cpuinfo | grep 'processor' | wc -l)
	else
		threads=$(nvram get ss_threads)
	fi
	logger -t "SS" "启动$stype主服务器..."
	case "$stype" in
	ss | ssr)
		last_config_file=$CONFIG_FILE
		pid_file="/tmp/ssr-retcp.pid"
		for i in $(seq 1 $threads); do
			$bin -c $CONFIG_FILE $ARG_OTA -f /tmp/ssr-retcp_$i.pid >/dev/null 2>&1
			usleep 500000
		done
		redir_tcp=1
		echo "$(date "+%Y-%m-%d %H:%M:%S") Shadowsocks/ShadowsocksR $threads 线程启动成功!" >>/tmp/ssrplus.log
		;;
	trojan)
		for i in $(seq 1 $threads); do
			$bin --config $trojan_json_file >>/tmp/ssrplus.log 2>&1 &
			usleep 500000
		done
		echo "$(date "+%Y-%m-%d %H:%M:%S") $($bin --version 2>&1 | head -1) Started!" >>/tmp/ssrplus.log
		;;
	v2ray)
		$bin -config $v2_json_file >/dev/null 2>&1 &
		echo "$(date "+%Y-%m-%d %H:%M:%S") $($bin -version | head -1) 启动成功!" >>/tmp/ssrplus.log
		;;
	xray)
		$bin -config $v2_json_file >/dev/null 2>&1 &
		echo "$(date "+%Y-%m-%d %H:%M:%S") $($bin -version | head -1) 启动成功!" >>/tmp/ssrplus.log
		;;	
	socks5)
		for i in $(seq 1 $threads); do
		lua /etc_ro/ss/gensocks.lua $GLOBAL_SERVER 1080 >/dev/null 2>&1 &
		usleep 500000
		done
	    ;;
	esac
	return 0
	}
	
start_redir_udp() {
	if [ "$UDP_RELAY_SERVER" != "nil" ]; then
		redir_udp=1
		logger -t "SS" "启动$utype游戏UDP中继服务器"
		utype=$(nvram get ud_type)
		local bin=$(find_bin $utype)
		[ ! -f "$bin" ] && echo "$(date "+%Y-%m-%d %H:%M:%S") UDP TPROXY Relay:Can't find $bin program, can't start!" >>/tmp/ssrplus.log && return 1
		case "$utype" in
		ss | ssr)
			ARG_OTA=""
			gen_config_file $UDP_RELAY_SERVER 1 1080
			last_config_file=$CONFIG_UDP_FILE
			pid_file="/var/run/ssr-reudp.pid"
			$bin -c $last_config_file $ARG_OTA -U -f /var/run/ssr-reudp.pid >/dev/null 2>&1
			;;
		v2ray)
			gen_config_file $UDP_RELAY_SERVER 1
			$bin -config /tmp/v2-ssr-reudp.json >/dev/null 2>&1 &
			;;
		xray)
			gen_config_file $UDP_RELAY_SERVER 1
			$bin -config /tmp/v2-ssr-reudp.json >/dev/null 2>&1 &
			;;	
		trojan)
			gen_config_file $UDP_RELAY_SERVER 1
			$bin --config /tmp/trojan-ssr-reudp.json >/dev/null 2>&1 &
			ipt2socks -U -b 0.0.0.0 -4 -s 127.0.0.1 -p 10801 -l 1080 >/dev/null 2>&1 &
			;;
		socks5)
		echo "1"
		    ;;
		esac
	fi
	return 0
	}
	ss_switch=$(nvram get backup_server)
	if [ $ss_switch != "nil" ]; then
		switch_time=$(nvram get ss_turn_s)
		switch_timeout=$(nvram get ss_turn_ss)
		#/usr/bin/ssr-switch start $switch_time $switch_timeout &
		socks="-o"
	fi
	#return $?



start_dns() {
case "$run_mode" in
	router)
		echo "create china hash:net family inet hashsize 1024 maxelem 65536" >/tmp/china.ipset
		awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/storage/chinadns/chnroute.txt >>/tmp/china.ipset
		ipset -! flush china
		ipset -! restore </tmp/china.ipset 2>/dev/null
		rm -f /tmp/china.ipset
		if [ $(nvram get ss_chdns) = 1 ]; then
			chinadnsng_enable_flag=1
			logger -t "SS" "下载cdn域名文件..."
			wget --no-check-certificate --timeout=8 -qO - https://gitee.com/bkye/rules/raw/master/cdn.txt > /tmp/cdn.txt
			if [ ! -f "/tmp/cdn.txt" ]; then
				logger -t "SS" "cdn域名文件下载失败，可能是地址失效或者网络异常！可能会影响部分国内域名解析了国外的IP！"
			else
				logger -t "SS" "cdn域名文件下载成功"
			fi
			logger -st "SS" "启动chinadns..."
			dns2tcp -L"127.0.0.1#5353" -R"$(nvram get tunnel_forward)" >/dev/null 2>&1 &
			chinadns-ng -b 0.0.0.0 -l 65353 -c $(nvram get china_dns) -t 127.0.0.1#5353 -4 china -M -m /tmp/cdn.txt >/dev/null 2>&1 &
			sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
			sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
			cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
no-resolv
server=127.0.0.1#65353
EOF
    		fi
	;;
	gfw)
		if [ $(nvram get pdnsd_enable) = 0 ]; then
			dnsstr="$(nvram get tunnel_forward)"
			dnsserver=$(echo "$dnsstr" | awk -F '#' '{print $1}')
			#dnsport=$(echo "$dnsstr" | awk -F '#' '{print $2}')
			ipset add gfwlist $dnsserver 2>/dev/null
			logger -st "SS" "启动dns2tcp：5353端口..."
			dns2tcp -L"127.0.0.1#5353" -R"$dnsstr" >/dev/null 2>&1 &
			pdnsd_enable_flag=0	
			logger -st "SS" "开始处理gfwlist..."
		fi
		;;
	oversea)
		ipset add gfwlist $dnsserver 2>/dev/null
		mkdir -p /etc/storage/dnsmasq.oversea
		sed -i '/dnsmasq-ss/d' /etc/storage/dnsmasq/dnsmasq.conf
		sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
		cat >>/etc/storage/dnsmasq/dnsmasq.conf <<EOF
conf-dir=/etc/storage/dnsmasq.oversea
EOF
;;
	*)
		ipset -N ss_spec_wan_ac hash:net 2>/dev/null
		ipset add ss_spec_wan_ac $dnsserver 2>/dev/null
	;;
	esac
	/sbin/restart_dhcpd
}

start_AD() {
	mkdir -p /tmp/dnsmasq.dom
	curl -k -s -o /tmp/adnew.conf --connect-timeout 10 --retry 3 $(nvram get ss_adblock_url)
	if [ ! -f "/tmp/adnew.conf" ]; then
		logger -t "SS" "AD文件下载失败，可能是地址失效或者网络异常！"
	else
		logger -t "SS" "AD文件下载成功"
		if [ -f "/tmp/adnew.conf" ]; then
			check = `grep -wq "address=" /tmp/adnew.conf`
	  		if [ ! -n "$check" ] ; then
	    			cp /tmp/adnew.conf /tmp/dnsmasq.dom/ad.conf
	  		else
			    cat /tmp/adnew.conf | grep ^\|\|[^\*]*\^$ | sed -e 's:||:address\=\/:' -e 's:\^:/0\.0\.0\.0:' > /tmp/dnsmasq.dom/ad.conf
			fi
		fi
	fi
	rm -f /tmp/adnew.conf
}


# ================================= 启动 Socks5代理 ===============================
start_local() {
	local s5_port=$(nvram get socks5_port)
	local local_server=$(nvram get socks5_enable)
	[ "$local_server" == "nil" ] && return 1
	[ "$local_server" == "same" ] && local_server=$GLOBAL_SERVER
	local type=$(nvram get s5_type)
	local bin=$(find_bin $type)
	[ ! -f "$bin" ] && echo "$(date "+%Y-%m-%d %H:%M:%S") Global_Socks5:Can't find $bin program, can't start!" >>/tmp/ssrplus.log && return 1
	case "$type" in
	ss | ssr)
		local name="Shadowsocks"
		local bin=$(find_bin ss-local)
		[ ! -f "$bin" ] && echo "$(date "+%Y-%m-%d %H:%M:%S") Global_Socks5:Can't find $bin program, can't start!" >>/tmp/ssrplus.log && return 1
		[ "$type" == "ssr" ] && name="ShadowsocksR"
		gen_config_file $local_server 3 $s5_port
		$bin -c $CONFIG_SOCK5_FILE -u -f /var/run/ssr-local.pid >/dev/null 2>&1
		echo "$(date "+%Y-%m-%d %H:%M:%S") Global_Socks5:$name Started!" >>/tmp/ssrplus.log
		;;
	v2ray)
		lua /etc_ro/ss/genv2config.lua $local_server tcp 0 $s5_port >/tmp/v2-ssr-local.json
		sed -i 's/\\//g' /tmp/v2-ssr-local.json
		$bin -config /tmp/v2-ssr-local.json >/dev/null 2>&1 &
		echo "$(date "+%Y-%m-%d %H:%M:%S") Global_Socks5:$($bin -version | head -1) Started!" >>/tmp/ssrplus.log
		;;
	xray)
		lua /etc_ro/ss/genxrayconfig.lua $local_server tcp 0 $s5_port >/tmp/v2-ssr-local.json
		sed -i 's/\\//g' /tmp/v2-ssr-local.json
		$bin -config /tmp/v2-ssr-local.json >/dev/null 2>&1 &
		echo "$(date "+%Y-%m-%d %H:%M:%S") Global_Socks5:$($bin -version | head -1) Started!" >>/tmp/ssrplus.log
		;;
	trojan)
		lua /etc_ro/ss/gentrojanconfig.lua $local_server client $s5_port >/tmp/trojan-ssr-local.json
		sed -i 's/\\//g' /tmp/trojan-ssr-local.json
		$bin --config /tmp/trojan-ssr-local.json >/dev/null 2>&1 &
		echo "$(date "+%Y-%m-%d %H:%M:%S") Global_Socks5:$($bin --version 2>&1 | head -1) Started!" >>/tmp/ssrplus.log
		;;
	*)
		[ -e /proc/sys/net/ipv6 ] && local listenip='-i ::'
		microsocks $listenip -p $s5_port ssr-local >/dev/null 2>&1 &
		echo "$(date "+%Y-%m-%d %H:%M:%S") Global_Socks5:$type Started!" >>/tmp/ssrplus.log
		;;
	esac
	local_enable=1
	return 0
}

rules() {
	[ "$GLOBAL_SERVER" = "nil" ] && return 1
	UDP_RELAY_SERVER=$(nvram get udp_relay_server)
	if [ "$UDP_RELAY_SERVER" = "same" ]; then
	UDP_RELAY_SERVER=$GLOBAL_SERVER
	fi
	if start_rules; then
		return 0
	else
		return 1
	fi
}

start_watchcat() {
	if [ $(nvram get ss_watchcat) = 1 ]; then
		let total_count=server_count+redir_tcp+redir_udp+tunnel_enable+v2ray_enable+local_enable+pdnsd_enable_flag+chinadnsng_enable_flag
		if [ $total_count -gt 0 ]; then
			#param:server(count) redir_tcp(0:no,1:yes)  redir_udp tunnel kcp local gfw
			/usr/bin/ssr-monitor $server_count $redir_tcp $redir_udp $tunnel_enable $v2ray_enable $local_enable $pdnsd_enable_flag $chinadnsng_enable_flag >/dev/null 2>&1 &
		fi
	fi
}

auto_update() {
	sed -i '/update_chnroute/d' /etc/storage/cron/crontabs/$http_username
	sed -i '/update_gfwlist/d' /etc/storage/cron/crontabs/$http_username
	sed -i '/ss-watchcat/d' /etc/storage/cron/crontabs/$http_username
	if [ $(nvram get ss_update_chnroute) = "1" ]; then
		cat >>/etc/storage/cron/crontabs/$http_username <<EOF
0 8 */10 * * /usr/bin/update_chnroute.sh > /dev/null 2>&1
EOF
	fi
	if [ $(nvram get ss_update_gfwlist) = "1" ]; then
		cat >>/etc/storage/cron/crontabs/$http_username <<EOF
0 7 */10 * * /usr/bin/update_gfwlist.sh > /dev/null 2>&1
EOF
	fi
}

# ================================= 启动 SS ===============================
ssp_start() { 
    ss_enable=`nvram get ss_enable`
if rules; then
		if start_redir_tcp; then
		start_redir_udp
        #start_rules
		#start_AD
        start_dns
		fi
		fi
        start_local
        start_watchcat
        auto_update
        ENABLE_SERVER=$(nvram get global_server)
        [ "$ENABLE_SERVER" = "-1" ] && return 1

        logger -t "SS" "启动成功。"
        logger -t "SS" "内网IP控制为:$lancons"
        nvram set check_mode=0
}

# ================================= 关闭SS ===============================

ssp_close() {
	rm -rf /tmp/cdn
	/usr/bin/ss-rules -f
	kill -9 $(ps | grep ssr-switch | grep -v grep | awk '{print $1}') >/dev/null 2>&1
	kill -9 $(ps | grep ssr-monitor | grep -v grep | awk '{print $1}') >/dev/null 2>&1
	kill_process
	sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/cdn/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/gfwlist/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
	if [ -f "/etc/storage/dnsmasq-ss.d" ]; then
		rm -f /etc/storage/dnsmasq-ss.d
	fi
	clear_iptable
	/sbin/restart_dhcpd
}


clear_iptable()
{
	s5_port=$(nvram get socks5_port)
	iptables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT
	iptables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT
	ip6tables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT
	ip6tables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT
	
}

kill_process() {
	v2ray_process=$(pidof v2ray)
	if [ -n "$v2ray_process" ]; then
		logger -t "SS" "关闭V2Ray进程..."
		killall v2ray >/dev/null 2>&1
		kill -9 "$v2ray_process" >/dev/null 2>&1
	fi
	ssredir=$(pidof ss-redir)
	if [ -n "$ssredir" ]; then
		logger -t "SS" "关闭ss-redir进程..."
		killall ss-redir >/dev/null 2>&1
		kill -9 "$ssredir" >/dev/null 2>&1
	fi

	rssredir=$(pidof ssr-redir)
	if [ -n "$rssredir" ]; then
		logger -t "SS" "关闭ssr-redir进程..."
		killall ssr-redir >/dev/null 2>&1
		kill -9 "$rssredir" >/dev/null 2>&1
	fi
	
	sslocal_process=$(pidof ss-local)
	if [ -n "$sslocal_process" ]; then
		logger -t "SS" "关闭ss-local进程..."
		killall ss-local >/dev/null 2>&1
		kill -9 "$sslocal_process" >/dev/null 2>&1
	fi

	trojandir=$(pidof trojan)
	if [ -n "$trojandir" ]; then
		logger -t "SS" "关闭trojan进程..."
		killall trojan >/dev/null 2>&1
		kill -9 "$trojandir" >/dev/null 2>&1
	fi

	kumasocks_process=$(pidof kumasocks)
	if [ -n "$kumasocks_process" ]; then
		logger -t "SS" "关闭kumasocks进程..."
		killall kumasocks >/dev/null 2>&1
		kill -9 "$kumasocks_process" >/dev/null 2>&1
	fi
	
	ipt2socks_process=$(pidof ipt2socks)
	if [ -n "$ipt2socks_process" ]; then
		logger -t "SS" "关闭ipt2socks进程..."
		killall ipt2socks >/dev/null 2>&1
		kill -9 "$ipt2socks_process" >/dev/null 2>&1
	fi

	socks5_process=$(pidof srelay)
	if [ -n "$socks5_process" ]; then
		logger -t "SS" "关闭socks5进程..."
		killall srelay >/dev/null 2>&1
		kill -9 "$socks5_process" >/dev/null 2>&1
	fi

	ssrs_process=$(pidof ssr-server)
	if [ -n "$ssrs_process" ]; then
		logger -t "SS" "关闭ssr-server进程..."
		killall ssr-server >/dev/null 2>&1
		kill -9 "$ssrs_process" >/dev/null 2>&1
	fi
	
	cnd_process=$(pidof chinadns-ng)
	if [ -n "$cnd_process" ]; then
		logger -t "SS" "关闭chinadns-ng进程..."
		killall chinadns-ng >/dev/null 2>&1
		kill -9 "$cnd_process" >/dev/null 2>&1
	fi

	dns2tcp_process=$(pidof dns2tcp)
	if [ -n "$dns2tcp_process" ]; then
		logger -t "SS" "关闭dns2tcp进程..."
		killall dns2tcp >/dev/null 2>&1
		kill -9 "$dns2tcp_process" >/dev/null 2>&1
	fi
	
	microsocks_process=$(pidof microsocks)
	if [ -n "$microsocks_process" ]; then
		logger -t "SS" "关闭socks5服务端进程..."
		killall microsocks >/dev/null 2>&1
		kill -9 "$microsocks_process" >/dev/null 2>&1
	fi
}


# ================================= 重启 SS ===============================
ressp() {
	BACKUP_SERVER=$(nvram get backup_server)
	start_redir $BACKUP_SERVER
	start_rules $BACKUP_SERVER
	start_dns
	start_local
	start_watchcat
	auto_update
	ENABLE_SERVER=$(nvram get global_server)
	logger -t "SS" "备用服务器启动成功"
	logger -t "SS" "内网IP控制为:$lancons"
}

case $1 in
start)
	ssp_start
	;;
stop)
	killall -q -9 ssr-switch
	ssp_close
	;;
restart)
	ssp_close
	ssp_start
	;;
reserver)
	ssp_close
	ressp
	;;
*)
	echo "check"
	#exit 0
	;;
esac

