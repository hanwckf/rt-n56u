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
v2udp_json_file="/tmp/v2-udpredir.json"
trojan_json_file="/tmp/tj-redir.json"
server_count=0
redir_tcp=0
v2ray_enable=0
redir_udp=0
tunnel_enable=0
local_enable=0
pdnsd_enable_flag=0
wan_bp_ips="/tmp/whiteip.txt"
wan_fw_ips="/tmp/blackip.txt"
lan_fp_ips="/tmp/lan_ip.txt"
run_mode=`nvram get ss_run_mode`
ss_turn=`nvram get ss_turn`
ss_udp=`nvram get ss_udp`
lan_con=`nvram get lan_con`
ss_own=`nvram get ss_own`
socks=""

if [ $ss_own = "1" ]; then
   socks="-o"
fi
gen_config_file() {
    hostip=$(nvram get ssp_server_x$1)
	config_file=$CONFIG_FILE
	fastopen="false"
	if [ $(nvram get ss_list) = 0 ]; then
		stype=$(nvram get ssp_type_x$1)
	else
		stype=$(nvram get d_type)
	fi
	logger -t "SS" "正在创建$stype客户端的json文件..."
	if [ "$stype" == "ss" ]; then
		cat <<-EOF >$config_file
			{
				"server": "$hostip",
				"server_port": $(nvram get ssp_prot_x$1),
				"local_address": "0.0.0.0",
				"local_port": $(nvram get ssp_local_port),
				"password": "$(nvram get ss_key_x$1)",
				"timeout": 60,
				"method": "$(nvram get ss_method_x$1)",
				"plugin": "$(nvram get ss_plugin_x$1)",
				"reuse_port": true
			}
		EOF
	elif [ "$stype" == "ssr" ]; then
		if [ $(nvram get ss_list) = 1 ]; then
			ssr_server=$(nvram get d_server)
			ssr_prot=$(nvram get d_port)
			ssr_local_port=$(nvram get ssp_local_port)
			ssr_key=$(nvram get d_ss_password)
			ssr_method=$(nvram get d_ss_method)
			ssr_protocol=$(nvram get d_ss_protocol)
			ssr_proto_param=$(nvram get d_ss_protoparam)
			ssr_obfs=$(nvram get d_ss_obfs)
			ssr_obfs_param=$(nvram get d_ss_obfsparam)
		else
			ssr_server=$hostip
			ssr_prot=$(nvram get ssp_prot_x$1)
			ssr_local_port=$(nvram get ssp_local_port)
			ssr_key=$(nvram get ss_key_x$1)
			ssr_method=$(nvram get ss_method_x$1)
			ssr_protocol=$(nvram get ss_protocol_x$1)
			ssr_proto_param=$(nvram get ss_proto_param_x$1)
			ssr_obfs=$(nvram get ss_obfs_x$1)
			ssr_obfs_param=$(nvram get ss_obfs_param_x$1)
		fi

		cat <<-EOF >$config_file
			{
				"server": "$ssr_server",
				"server_port": $ssr_prot,
				"local_address": "0.0.0.0",
				"local_port": $ssr_local_port,
				"password": "$ssr_key",
				"timeout": 60,
				"method": "$ssr_method",
				"protocol": "$ssr_protocol",
				"protocol_param": "$ssr_proto_param",
				"obfs": "$ssr_obfs",
				"obfs_param": "$ssr_obfs_param",
				"reuse_port": true
			}
		EOF
	elif [ "$stype" == "kumasocks" ]; then
		kumasocks_bin="/usr/bin/kumasocks"
		cat <<-EOF >$CONFIG_KUMASOCKS_FILE
			listen-addr = "0.0.0.0:$(nvram get ssp_local_port)"
			proxy-addr = "socks5://$hostip:$(nvram get ssp_prot_x$1)"
		EOF
	elif [ "$stype" == "trojan" ]; then
		tj_bin="/usr/bin/trojan"
		if [ ! -f "$tj_bin" ]; then
			curl -k -s -o /tmp/trojan --connect-timeout 10 --retry 3 https://cdn.jsdelivr.net/gh/chongshengB/rt-n56u/trunk/user/trojan/trojan
			if [ ! -f "/tmp/trojan" ]; then
				logger -t "SS" "trojan二进制文件下载失败，可能是地址失效或者网络异常！"
				nvram set ss_enable=0
				ssp_close
			else
				logger -t "SS" "trojan二进制文件下载成功"
				chmod -R 777 /tmp/trojan
				tj_bin="/tmp/trojan"
			fi
		fi
		if [ $(nvram get v2_tls_x$1) = "1" ]; then
			tj_link_tls="true"
			tj_link_tls_host=$(nvram get tj_tls_host_x$1)
		else
			tj_link_tls="false"
			tj_link_tls_host=""
		fi
		#tj_file=$trojan_json_file
		cat <<-EOF >$trojan_json_file
			{
	    "run_type": "nat",
	    "local_addr": "0.0.0.0",
	    "local_port": $(nvram get ssp_local_port),
	    "remote_addr": "$hostip",
	    "remote_port": $(nvram get ssp_prot_x$1),
	    "password": [
	        "$(nvram get ss_key_x$1)"
	    ],
	    "log_level": 99,
	    "ssl": {
	        "verify": false,
	        "verify_hostname": $tj_link_tls,
	        "cert": "",
	        "cipher_tls13": "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384",
	        "sni": "$tj_link_tls_host",
	        "alpn": [
	            "h2",
	            "http/1.1"
	        ],
	        "reuse_session": true,
	        "session_ticket": false,
	        "curves": ""
	    },
	    "tcp": {
	        "no_delay": true,
	        "keep_alive": true,
			"reuse_port": true,
	        "fast_open": false,
	        "fast_open_qlen": 20
	    }
	}
		EOF
	elif [ "$stype" == "v2ray" ]; then
		v2_file=$v2_json_file
		v2_bin="/usr/bin/v2ray"
		if [ ! -f "$v2_bin" ]; then
			curl -k -s -o /tmp/v2ray --connect-timeout 10 --retry 3 https://cdn.jsdelivr.net/gh/chongshengB/rt-n56u/trunk/user/v2ray/v2ray
			if [ ! -f "/tmp/v2ray" ]; then
				logger -t "SS" "v2ray二进制文件下载失败，可能是地址失效或者网络异常！"
				nvram set ss_enable=0
				ssp_close
			else
				logger -t "SS" "v2ray二进制文件下载成功"
				chmod -R 777 /tmp/v2ray
				v2_bin="/tmp/v2ray"
			fi
		fi
		v2ray_enable=1
		#创建v2ray json文件的代码用的是hiboyhiboy的,特此感谢
		if [ $(nvram get ss_list) = 0 ]; then
			vmess_link_add=$hostip
			vmess_link_port=$(nvram get ssp_prot_x$1)
			vmess_link_id=$(nvram get v2_vid_x$1)
			vmess_link_aid=$(nvram get v2_aid_x$1)
			vmess_link_net=$(nvram get v2_net_x$1)
			vmess_link_security=$(nvram get v2_security_x$1)
			if [ $(nvram get v2_tls_x$1) = "1" ]; then
				vmess_link_tls="tls"
			else
				vmess_link_tls="none"
			fi
			vmess_link_type_tcp=$(nvram get v2_type_tcp_x$1)
			vmess_link_type_kcp=$(nvram get v2_type_mkcp_x$1)
			vmess_link_webs_path=$(nvram get v2_webs_path_x$1)
			vmess_link_webs_host=$(nvram get v2_webs_host_x$1)
			vmess_link_http2_path=$(nvram get v2_http2_path_x$1)
			vmess_link_http2_host=$(nvram get v2_http2_host_x$1)
		else
			vmess_link_add=$(nvram get d_server)
			vmess_link_port=$(nvram get d_port)
			vmess_link_id=$(nvram get d_v2_uid)
			vmess_link_aid=$(nvram get d_v2_aid)
			vmess_link_net=$(nvram get d_v2_net)
			vmess_link_security=$(nvram get d_v2_security)
			vmess_link_tls=$(nvram get d_v2_tls)
			if [ "$vmess_link_net" = "tcp" ]; then
				vmess_link_type_tcp=$(nvram get d_v2_type)
				vmess_link_type_tcp_host=$(nvram get d_v2_host)
			fi
			if [ "$vmess_link_net" = "kcp" ]; then
				vmess_link_type_kcp=$(nvram get d_v2_type)
			fi
			if [ "$vmess_link_net" = "ws" ]; then
				vmess_link_webs_path=$(nvram get d_v2_path)
				vmess_link_webs_host=$(nvram get d_v2_host)
			fi
			if [ "$vmess_link_net" = "h2" ]; then
				vmess_link_http2_path=$(nvram get d_v2_path)
				vmess_link_http2_host=$(nvram get d_v2_host)
			fi

		fi

		mk_vmess=$(json_int_vmess_settings)
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"address"];"'$vmess_link_add'")')
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"users",0,"alterId"];'$vmess_link_aid')')
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"users",0,"id"];"'$vmess_link_id'")')
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"users",0,"security"];"'$vmess_link_security'")')
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"port"];'$vmess_link_port')')
		vmess_settings=$mk_vmess
		mk_vmess=$(json_int_vmess_streamSettings)
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["network"];"'$vmess_link_net'")')
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["security"];"'$vmess_link_tls'")')
		if [ "$vmess_link_tls" = "tls" ]; then
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["tlsSettings","serverName"];"'$vmess_link_webs_host'")')
		fi
		# tcp star
		if [ "$vmess_link_net" = "tcp" ]; then
			[ ! -z "$vmess_link_type_tcp" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["tcpSettings","type"];"'$vmess_link_type_tcp'")')
			#vmess_link_path=$(echo $vmess_link_path | sed 's/,/ /g')
			#link_path_i=0
			#for link_path in $vmess_link_path
			#do
			#	mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["tcpSettings","request","path",'$link_path_i'];"'$link_path'")')
			#	link_path_i=$(( link_path_i + 1 ))
			#done
			vmess_link_host=$(echo $vmess_link_type_tcp_host | sed 's/,/ /g')
			link_host_i=0
			for link_host in $vmess_link_host; do
				mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["tcpSettings","request","headers","Host",'$link_host_i'];"'$link_host'")')
				link_host_i=$((link_host_i + 1))
			done
		fi
		# tcp end
		# kcp star
		if [ "$vmess_link_net" = "kcp" ]; then
			[ ! -z "$vmess_link_type_kcp)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["kcpSettings","header","type"];"'$vmess_link_type_kcp'")')
		fi
		# kcp end
		# ws star
		if [ "$vmess_link_net" = "ws" ]; then
			[ ! -z "$vmess_link_webs_path" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["wsSettings","path"];"'$vmess_link_webs_path'")')
			[ ! -z "$vmess_link_webs_host" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["wsSettings","headers","Host"];"'$vmess_link_webs_host'")')
		fi
		# ws end
		# h2 star
		if [ "$vmess_link_net" = "h2" ]; then
			[ ! -z "$vmess_link_http2_path" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["httpSettings","path"];"'$vmess_link_http2_path'")')
			vmess_link_host=$(echo $vmess_link_http2_host | sed 's/,/ /g')
			link_host_i=0
			for link_host in $vmess_link_host; do
				mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["httpSettings","host",'$link_host_i'];"'$link_host'")')
				link_host_i=$((link_host_i + 1))
			done
		fi
		# h2 end
		# quic star
		if [ "$vmess_link_net" = "quic" ]; then
			[ ! -z "$(nvram get v2_quic_header_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["quicSettings","header","type"];"'$(nvram get v2_quic_header_x$1)'")')
			[ ! -z "$(nvram get v2_quic_security_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["quicSettings","security"];"'$(nvram get v2_quic_security_x$1)'")')
			[ ! -z "$(nvram get v2_quic_key_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["quicSettings","key"];"'$(nvram get v2_quic_key_x$1)'")')
		fi
		# quic end
		vmess_streamSettings=$mk_vmess
		mk_vmess=$(json_int)
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["outbounds",0,"settings"];'"$vmess_settings"')')
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["outbounds",0,"streamSettings"];'"$vmess_streamSettings"')')
		mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["outbounds",0,"protocol"];"vmess")')
		#if [ $ss_udp = "1" ];then
		#mk_vmess=$(echo $mk_vmess| jq --raw-output 'setpath(["inbounds",0,"settings","udp"];'true')')
		#fi
		echo $mk_vmess | jq --raw-output '.' >$v2_file
	#创建json文件结束
	fi
}

start_rules() {
    logger -t "SS" "正在添加防火墙规则..."
	if [ $(nvram get ss_list) = 0 ]; then
		server=$(nvram get ssp_server_x$1)
	else
		server=$(nvram get d_server)
	fi
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
	router_proxy="1"
	if [ "$ss_udp" = 1 ]; then
		ARG_UDP="-u"
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
	-k "$lancon" \
	$socks $gfwmode $ARG_UDP
	return $?
}


###############PDNSD
start_pdnsd() {
	pdnsd_bin="/usr/bin/pdnsd"
	pdnsd_cache="/tmp/pdnsd"
	pdnsd_file="/tmp/pdnsd.conf"
	pdnsd_pid="/tmp/pdnsd.pid"
	usr_dns="$1"
	usr_port="$2"
	tcp_dns_list="208.67.222.222, 208.67.220.220"
	[ -z "$usr_dns" ] && usr_dns="8.8.4.4"
	[ -z "$usr_port" ] && usr_port="53"
	dnsd_enable=$(nvram get pdnsd_enable)
	if [ $dnsd_enable = 0 ]; then
		if [ ! -d $pdnsd_cache ]; then
			mkdir -p $pdnsd_cache
			echo -ne "pd13\000\000\000\000" >$pdnsd_cache/pdnsd.cache
			chown -R nobody:nogroup $pdnsd_cache
		fi
		cat >$pdnsd_file <<EOF
global {
perm_cache=512;
cache_dir="$pdnsd_cache";
pid_file = $pdnsd_pid;
run_as="nobody";
server_port = 5353;
server_ip = 127.0.0.1;
status_ctl = on;
query_method=tcp_only;
min_ttl=1m;
max_ttl=1w;
timeout=5;
}
server {
label= "ssr-usedns";
ip = $usr_dns;
port = $usr_port;
timeout=6;
uptest=none;
interval=10m;
purge_cache=off;
}
server {
label= "opendns";
ip = $tcp_dns_list;
port = 443;
timeout=6;
uptest=none;
interval=10m;
purge_cache=off;
}
EOF
		chmod 600 $pdnsd_file
		logger -t "SS" "正在启动pdnsd..."
		$pdnsd_bin -c $pdnsd_file -d
	fi
}
###############PDNSD

start_redir() {
	ARG_OTA=""
	if [ $(nvram get ss_list) = 0 ]; then
		gen_config_file $1 0
		stype=$(nvram get ssp_type_x$1)
	else
		gen_config_file
		stype=$(nvram get d_type)
	fi
	logger -t "SS" "正在启动$stype程序..."
	if [ "$stype" == "ss" ]; then
		sscmd="ss-redir"
	elif [ "$stype" == "ssr" ]; then
		sscmd="ssr-redir"
	elif [ "$stype" == "trojan" ]; then
		sscmd="$tj_bin"
	elif [ "$stype" == "kumasocks" ]; then
		sscmd="$kumasocks_bin"
	elif [ "$stype" == "v2ray" ]; then
		sscmd="$v2_bin"
	elif [ "$stype" == "socks5" ]; then
		sscmd="/usr/bin/ipt2socks"
	fi
	if [ "$(nvram get ss_threads)" = "0" ]; then
		threads=$(cat /proc/cpuinfo | grep 'processor' | wc -l)
	else
		threads=$(nvram get ss_threads)
	fi
	if [ "$stype" == "ss" -o "$stype" == "ssr" ]; then
		last_config_file=$CONFIG_FILE
		pid_file="/tmp/ssr-retcp.pid"
		for i in $(seq 1 $threads); do
			$sscmd -c $CONFIG_FILE $ARG_OTA -f /tmp/ssr-retcp_$i.pid >/dev/null 2>&1
			logger -t "SS" "启动$stype第$i线程..."
			usleep 500000
		done
		redir_tcp=1
		echo "$(date "+%Y-%m-%d %H:%M:%S") Shadowsocks/ShadowsocksR $threads 线程启动成功!" >>/tmp/ssrplus.log
	elif [ "$stype" == "trojan" ]; then
		for i in $(seq 1 $threads); do
			$sscmd --config $trojan_json_file >>/tmp/ssrplus.log 2>&1 &
			logger -t "SS" "启动$stype第$i线程..."
			usleep 500000
		done
		echo "$(date "+%Y-%m-%d %H:%M:%S") $($sscmd --version 2>&1 | head -1) Started!" >>/tmp/ssrplus.log
	elif [ "$stype" == "socks5" ]; then
	if [ $(nvram get s5_username_x$1) != "" ]; then
	unp="-a $(nvram get s5_username_x$1)"
	if [ $(nvram get s5_password_x$1) != "" ]; then
	unp="$unp -k $(nvram get s5_password_x$1)"
	fi
	fi
	for i in $(seq 1 $threads); do
$sscmd -T -4 -b 0.0.0.0 -s $(nvram get ssp_server_x$1) -p $(nvram get ssp_prot_x$1) -l $(nvram get ssp_local_port) -R ssr-retcp $unp >/dev/null 2>&1 &done
  echo "$(date "+%Y-%m-%d %H:%M:%S") Socks5 REDIRECT/TPROXY, $threads Threads Started!" >>/tmp/ssrplus.log
	elif [ "$stype" == "kumasocks" ]; then
		$sscmd -c $CONFIG_KUMASOCKS_FILE &
	elif [ "$stype" == "v2ray" ]; then
		$sscmd -config $v2_json_file >/dev/null 2>&1 &
		echo "$(date "+%Y-%m-%d %H:%M:%S") $($sscmd -version | head -1) 启动成功!" >>/tmp/ssrplus.log
	fi
	ss_switch=$(nvram get backup_server)
	if [ $ss_switch != "nil" ]; then
		switch_time=$(nvram get ss_turn_s)
		switch_timeout=$(nvram get ss_turn_ss)
		/usr/bin/ssr-switch start $switch_time $switch_timeout &
		socks="-o"
	fi
	return $?
}


start_dns() {
	if [ "$run_mode" = "router" ]; then
		echo "create china hash:net family inet hashsize 1024 maxelem 65536" >/tmp/china.ipset
		awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/storage/chinadns/chnroute.txt >>/tmp/china.ipset
		ipset -! flush china
		ipset -! restore </tmp/china.ipset 2>/dev/null
		rm -f /tmp/china.ipset
#################PDNSD----------->
:<<!
		if [ $(nvram get pdnsd_enable) = 0 ]; then
			if [ $(nvram get sdns_enable) = 1 ]; then
				smart_process=$(pidof smartdns)
				if [ -n "$smart_process" ]; then
					logger -t "SS" "关闭smartdns进程..."
					/usr/bin/smartdns.sh stop
				fi
				nvram set sdns_enable=0
			fi
			dnsstr="$(nvram get tunnel_forward)"
			dnsserver=$(echo "$dnsstr" | awk -F ':' '{print $1}')
			dnsport=$(echo "$dnsstr" | awk -F ':' '{print $2}')
			start_pdnsd $dnsserver $dnsport
			pdnsd_enable_flag=1
			sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
			sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
			cat >>/etc/storage/dnsmasq/dnsmasq.conf <<EOF
no-resolv
server=127.0.0.1#5353
EOF

			mkdir -p /tmp/cdn
			logger -t "SS" "下载cdn域名文件..."
			wget --no-check-certificate --timeout=8 -qO - https://gitee.com/bkye/rules/raw/master/cdn.txt >/tmp/cdn.txt
			if [ ! -f "/tmp/cdn.txt" ]; then
				logger -t "SS" "cdn域名文件下载失败，可能是地址失效或者网络异常！"
			else
				logger -t "SS" "cdn域名文件下载成功"

				CDN="$(nvram get china_dns)"
				logger -t "SS" "正在使用绕过大陆IP模式，加载CDN列表用于国内域名走国内DNS解析。需要一点时间转换....."
				echo "#for china site CDN acclerate" >>/tmp/sscdn.conf
				cat /tmp/cdn.txt | sed "s/^/server=&\/./g" | sed "s/$/\/&$CDN/g" | sort | awk '{if ($0!=line) print;line=$0}' >>/tmp/cdn/sscdn.conf
				sed -i '/cdn/d' /etc/storage/dnsmasq/dnsmasq.conf
				cat >>/etc/storage/dnsmasq/dnsmasq.conf <<EOF
conf-dir=/tmp/cdn/
EOF
			fi
!
#<----------------################PDNSD
		if [ $(nvram get pdnsd_enable) = 1 ]; then
			if [ $(nvram get ssp_dns_ip) = 2 ]; then
				rm -f /tmp/whitelist.conf
				rm -f /tmp/smartdnsgfw.conf
				rm -f /tmp/smartdnschina.conf
				awk '{printf("whitelist-ip %s\n", $1, $1 )}' /etc/storage/chinadns/chnroute.txt >>/tmp/whitelist.conf
				cat >>/tmp/smartdnschina.conf <<EOF
server-name smartdns
bind :6053 -no-speed-check -no-dualstack-selection
bind-tcp :6053 -no-speed-check -no-dualstack-selection
cache-size 100
prefetch-domain yes
serve-expired yes
force-AAAA-SOA yes
log-level info
server 223.5.5.5:53 -whitelist-ip
server 119.29.29.29:53 -whitelist-ip
server 1.1.1.1:53
server 8.8.4.4:53
server-https https://1.1.1.1/dns-query
server-tls dns.google:53
conf-file /tmp/whitelist.conf
EOF
				/usr/bin/smartdns -f -c /tmp/smartdnschina.conf &>/dev/null &
				sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
				sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
				cat >>/etc/storage/dnsmasq/dnsmasq.conf <<EOF
no-resolv
server=127.0.0.1#6053
EOF
			elif [ $(nvram get ssp_dns_ip) = 0 ]; then
				/usr/bin/smartdns.sh stop
				/usr/bin/smartdns.sh start
			fi
		fi
	elif [ "$run_mode" = "gfw" ]; then
		logger -st "SS" "开始处理gfwlist..."
		mkdir -p /etc/storage/gfwlist/

		if [ $(nvram get pdnsd_enable) = 0 ]; then
			dnsstr="$(nvram get tunnel_forward)"
			dnsserver=$(echo "$dnsstr" | awk -F ':' '{print $1}')
			dnsport=$(echo "$dnsstr" | awk -F ':' '{print $2}')
			start_pdnsd $dnsserver $dnsport
			pdnsd_enable_flag=1
			ipset add gfwlist $dnsserver 2>/dev/null
			
		elif [ $(nvram get pdnsd_enable) = 1 ]; then
			if [ $(nvram get ssp_dns_ip) = 2 ]; then
				rm -f /tmp/whitelist.conf
				rm -f /tmp/smartdnsgfw.conf
				rm -f /tmp/smartdnschina.conf
				cat >>/tmp/smartdnsgfw.conf <<EOF
server-name smartdns
bind :6053 -no-speed-check -no-dualstack-selection
bind-tcp :6053 -no-speed-check -no-dualstack-selection
bind :5353  -no-speed-check -group gfwlist -no-rule-addr -no-rule-nameserver -no-rule-soa -no-dualstack-selection -no-cache
bind-tcp :5353 -no-speed-check -group gfwlist -no-rule-addr -no-rule-nameserver -no-rule-soa -no-dualstack-selection -no-cache
cache-size 100
prefetch-domain yes
serve-expired yes
force-AAAA-SOA yes
log-level info
server 223.5.5.5
server 119.29.29.29
server-tcp 8.8.8.8 -group gfwlist -exclude-default-group
server-tls dns.google  -group gfwlist -exclude-default-group
server-https https://ndns.233py.com/dns-query  -group gfwlist -exclude-default-group
EOF
				/usr/bin/smartdns -f -c /tmp/smartdnsgfw.conf &>/dev/null &
				ipset add gfwlist 8.8.8.8 2>/dev/null
				ipset add gfwlist dns.google 2>/dev/null
				ipset add gfwlist ndns.233py.com 2>/dev/null
				sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
				sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
				cat >>/etc/storage/dnsmasq/dnsmasq.conf <<EOF
no-resolv
server=127.0.0.1#6053
EOF
			elif [ $(nvram get ssp_dns_ip) = 0 ]; then
				/usr/bin/smartdns.sh stop
				/usr/bin/smartdns.sh start
			fi
		fi
	elif [ "$run_mode" = "oversea" ]; then
		ipset add gfwlist $dnsserver 2>/dev/null
		mkdir -p /etc/storage/dnsmasq.oversea
		sed -i '/dnsmasq-ss/d' /etc/storage/dnsmasq/dnsmasq.conf
		sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
		cat >>/etc/storage/dnsmasq/dnsmasq.conf <<EOF
conf-dir=/etc/storage/dnsmasq.oversea
EOF
	else
		ipset -N ss_spec_wan_ac hash:net 2>/dev/null
		ipset add ss_spec_wan_ac $dnsserver 2>/dev/null
	fi
	/sbin/restart_dhcpd
}



# ================================= 启动 Socks5代理 ===============================
start_local() {
	s5_enable=$(nvram get socks5_enable)
	s5_wenable=$(nvram get socks5_wenable)
	s5_aenable=$(nvram get socks5_aenable)
	s5_s_username=$(nvram get socks5_s_username)
	s5_s_password=$(nvram get socks5_s_password)
	s5_port=$(nvram get socks5_port)
	if [ "$s5_enable" = "1" ]; then
	if [ "$s5_aenable" = "1" ]; then
    microsocks -i 0.0.0.0 -p $s5_port -1 -u $s5_s_username -P $s5_s_password >/dev/null 2>&1 &
	else
    microsocks -i 0.0.0.0 -p $s5_port >/dev/null 2>&1 &
	fi
	if [ $s5_wenable = 1 ] || [ $s5_wenable = 3 ]; then
		fport=$(iptables -t filter -L INPUT -v -n --line-numbers | grep dpt:$s5_port | cut -d " " -f 1 | sort -nr | wc -l)
		if [ "$fport" = 0 ]; then
			iptables -t filter -I INPUT -p tcp --dport $s5_port -j ACCEPT
		fi
		logger -t "SS" "WAN IPV4放行 socks5 $s5_port tcp端口"
	fi
	if [ $s5_wenable = 2 ] || [ $s5_wenable = 3 ]; then
		f6port=$(ip6tables -t filter -L INPUT -v -n --line-numbers | grep dpt:$s5_port | cut -d " " -f 1 | sort -nr | wc -l)
		if [ "$f6port" = 0 ]; then
			ip6tables -t filter -I INPUT -p tcp --dport $s5_port -j ACCEPT
		fi
		logger -t "SS" "WAN IPV6放行 socks5 $s5_port tcp端口"
	fi
	fi
}


rules() {
    [ "$GLOBAL_SERVER" = "-1" ] && return 1
    [ "$UDP_RELAY_SERVER" = "same" ] && UDP_RELAY_SERVER=$GLOBAL_SERVER
    if start_rules $GLOBAL_SERVER;then
        return 0
    else
        return 1
    fi
}

start_watchcat() {
	if [ $(nvram get ss_watchcat) = 1 ]; then
		let total_count=server_count+redir_tcp+redir_udp+tunnel_enable+v2ray_enable+local_enable+pdnsd_enable_flag
		if [ $total_count -gt 0 ]; then
			#param:server(count) redir_tcp(0:no,1:yes)  redir_udp tunnel kcp local gfw
			/usr/bin/ssr-monitor $server_count $redir_tcp $redir_udp $tunnel_enable $v2ray_enable $local_enable $pdnsd_enable_flag >/dev/null 2>&1 &
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
    GLOBAL_SERVER=`nvram get global_server`
    echo $GLOBAL_SERVER
    ss_enable=`nvram get ss_enable`
    if [ $ss_enable != "0" ] && [ $GLOBAL_SERVER != "nil" ]; then
        start_redir $GLOBAL_SERVER
        start_rules $GLOBAL_SERVER
        start_dns
        start_local
        start_watchcat
        auto_update
        ENABLE_SERVER=$(nvram get global_server)
        [ "$ENABLE_SERVER" = "-1" ] && return 1
        logger -t "SS" "启动成功。"
        logger -t "SS" "内网IP控制为:$lancons"
        nvram set check_mode=0
    fi
}

# ================================= 关闭SS ===============================

ssp_close() {
	rm -rf /tmp/cdn
	/usr/bin/ss-rules -f
	kill -9 $(ps | grep ssr-switch | grep -v grep | awk '{print $1}') >/dev/null 2>&1
	kill -9 $(ps | grep ssr-monitor | grep -v grep | awk '{print $1}') >/dev/null 2>&1
	kill_process
	if [ $(nvram get sdns_enable) = 0 ]; then
		sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
		sed -i '/server=127.0.0.1#6053/d' /etc/storage/dnsmasq/dnsmasq.conf
		rm -f /tmp/whitelist.conf
		rm -f /tmp/smartdnsgfw.conf
		rm -f /tmp/smartdnschina.conf
	fi
	sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/server=127.0.0.1#5353/d' /etc/storage/dnsmasq/dnsmasq.conf
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
		logger -t "SS" "关闭V2Ray进程..."
		killall ss-redir >/dev/null 2>&1
		kill -9 "$ssredir" >/dev/null 2>&1
	fi

	rssredir=$(pidof ssr-redir)
	if [ -n "$rssredir" ]; then
		logger -t "SS" "关闭ssr-redir进程..."
		killall ssr-redir >/dev/null 2>&1
		kill -9 "$rssredir" >/dev/null 2>&1
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

	pdnsd_process=$(pidof pdnsd)
	if [ -n "$pdnsd_process" ]; then
		logger -t "SS" "关闭pdnsd进程..."
		killall pdnsd >/dev/null 2>&1
		kill -9 "$pdnsd_process" >/dev/null 2>&1
	fi
	
	microsocks_process=$(pidof microsocks)
	if [ -n "$microsocks_process" ]; then
		logger -t "SS" "关闭socks5服务端进程..."
		killall microsocks >/dev/null 2>&1
		kill -9 "$microsocks_process" >/dev/null 2>&1
	fi

	smart_process=$(pidof smartdns)
	if [ -n "$smart_process" ]; then
		if [ $(nvram get ssp_dns_ip) = 2 ]; then
			if [ $(nvram get sdns_enable) = 0 ]; then
				logger -t "SS" "关闭smartdns进程..."
				killall smartdns >/dev/null 2>&1
				kill -9 "$smart_process" >/dev/null 2>&1
			else
				if [ $(nvram get sdns_enable) = 1 ] && [ $(nvram get ss_enable) = 1 ]; then
					logger -t "SS" "关闭smartdns进程..."
					/usr/bin/smartdns.sh stop
					nvram set sdns_enable=0
				fi
			fi
		fi
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


# ================================= 配置文件 ===============================
json_int_trojan () {
echo '{
    "run_type": "nat",
    "local_addr": "0.0.0.0",
    "local_port": $(nvram get ssp_local_port_x$1),
    "remote_addr": "$hostip",
    "remote_port": $(nvram get ssp_prot_x$1),
    "password": [
        "$(nvram get ss_key_x$1)"
    ],
    "log_level": 1,
    "ssl": {
        "verify": false,
        "verify_hostname": false,
        "cert": "",
        "cipher_tls13": "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384",
        "sni": "",
        "alpn": [
            "h2",
            "http/1.1"
        ],
        "reuse_session": true,
        "session_ticket": false,
        "curves": ""
    },
    "tcp": {
        "no_delay": true,
        "keep_alive": true,
        "fast_open": false,
        "fast_open_qlen": 20
    }
}
'
}
json_int_vmess_settings () {
echo '{
"vnext": [
{
  "address": "127.0.0.1",
  "port": 37192,
  "users": [
	{
	  "id": "27848739-7e62-4138-9fd3-098a63964b6b",
	  "alterId": 4,
	  "security": "auto"
	}
  ]
}
]
}
'
}
json_int_vmess_streamSettings () {
echo '{
"network": "",
"security": "",
"tlsSettings": {
"allowInsecure": true,
 "serverName":" "
},
"tcpSettings": {
"type": "none",
"request": {
  "path": [
	"/"
  ],
  "headers": {
	"Host": []
  }
}
},
"kcpSettings": {
"header": {
  "type": "none"
}
},
"wsSettings": {
"path": "/",
"headers": {}
},
"httpSettings": {
"host": [
  "v2ray.com"
],
"path": "/"
},
"dsSettings": {},
"quicSettings": {
"security": "none",
"key": "",
"header": {
  "type": "none"
}
},
"sockopt": {
"mark": 255
}
}
'
}
json_int () {
echo '{
"log": {
"error": "/tmp/syslog.log",
"loglevel": "error"
},
"inbounds": [
{
  "port": "1080",
  "listen": "0.0.0.0",
  "protocol": "dokodemo-door",
  "settings": {
	"network": "tcp,udp",
	"timeout": 30,
	"followRedirect": true
  },
  "sniffing": {
	"enabled": true,
	"destOverride": [
	  "http",
	  "tls"
	]
  }
}
],
"outbounds": [
{
  "protocol": "",
  "settings": {},
  "streamSettings": {
	"network": "",
	"security": "",
	"tlsSettings": {},
	"tcpSettings": {},
	"kcpSettings": {},
	"wsSettings": {},
	"httpSettings": {},
	"dsSettings": {},
	"quicSettings": {},
	"sockopt": {
	  "mark": 255
	}
  }
},
{
  "protocol": "freedom",
  "settings": {},
  "tag": "direct",
  "streamSettings": {
	"sockopt": {
	  "mark": 255
	}
  }
},
{
  "protocol": "blackhole",
  "settings": {},
  "tag": "blocked",
  "streamSettings": {
	"sockopt": {
	  "mark": 255
	}
  }
}
]
}
'
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

