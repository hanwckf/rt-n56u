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
EXTRA_COMMANDS=rules
CONFIG_FILE=/tmp/${NAME}.json
CONFIG_UDP_FILE=/tmp/${NAME}_u.json
CONFIG_SOCK5_FILE=/tmp/${NAME}_s.json
v2_json_file="/tmp/v2-redir.json"
v2udp_json_file="/tmp/v2-udpredir.json"
server_count=0
redir_tcp=0
v2ray_enable=0
redir_udp=0
tunnel_enable=0
local_enable=0
kcp_enable_flag=0
kcp_flag=0
pdnsd_enable_flag=0
switch_enable=0
#switch_server=$1
threads=1
wan_bp_ips='/etc/storage/chinadns/chnroute.txt'
wan_fw_ips="/tmp/whileip.txt"
run_mode=`nvram get ss_run_mode`
ss_turn=`nvram get ss_turn`
gen_config_file() {
echo $2
logger -t "SS" "正在创建json文件..."
         host=`nvram get ssp_server_x$1`
         if echo $host|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then         
         hostip=${host}
         elif  [ "$host" != "${host#*:[0-9a-fA-F]}" ] ;then
         hostip=${host}
         else
          hostip=`ping ${host} -s 1 -c 1 | grep PING | cut -d'(' -f 2 | cut -d')' -f1`
          if echo $hostip|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then
          hostip=${hostip}
         else
          hostip=`cat /etc/storage/ssr_ip`
          fi
         fi
         [ $2 = "0" -a  $kcp_flag = "1" ] && hostip="127.0.0.1"
         
         if [ $2 = "0" ] ;then
         config_file=$CONFIG_FILE
         elif [ $2 = "1" ]; then
         config_file=$CONFIG_UDP_FILE
         else
         config_file=$CONFIG_SOCK5_FILE
         fi
         fastopen="false";
stype=`nvram get ssp_type_x$1`
if [ "$stype" == "ss" ] ;then
	cat <<-EOF >$config_file
{
		    "server": "$hostip",
		    "server_port": $(nvram get ssp_prot_x$1),
		    "local_address": "0.0.0.0",
		    "local_port": $(nvram get ssp_local_port_x$1),
		    "password": "$(nvram get ss_key_x$1)",
		    "timeout": 60,
		    "method": "$(nvram get ss_method_x$1)",
			"plugin": "$(nvram get ss_plugin_x$1)",
			"reuse_port": true,
		    "fast_open": $fastopen
}
EOF
       elif [ "$stype" == "ssr" ] ;then
	cat <<-EOF >$config_file
{
		    "server": "$hostip",
		    "server_port": $(nvram get ssp_prot_x$1),
		    "local_address": "0.0.0.0",
		    "local_port": $(nvram get ssp_local_port_x$1),
		    "password": "$(nvram get ss_key_x$1),",
		    "timeout": 60,
		    "method": "$(nvram get ss_method_x$1)",
		    "protocol": "$(nvram get ss_protocol_x$1)",
		    "protocol_param": "$(nvram get ss_proto_param_x$1)",
		    "obfs": "$(nvram get ss_obfs_x$1)",
		    "obfs_param": "$(nvram get ss_obfs_param_x$1)",
		    "reuse_port": true,
		    "fast_open": $fastopen
}
EOF
      elif [ "$stype" == "v2ray" ] ;then
	  v2_file=$v2_json_file
	  if [ $2 = "1" ] ;then
	  v2_file=$v2udp_json_file
	  fi
curl -k -s -o /tmp/v2ray --connect-timeout 10 --retry 3 https://dev.tencent.com/u/dtid_39de1afb676d0d78/p/kp/git/raw/master/v2ray
if [ ! -f "/tmp/v2ray" ]; then
logger -t "ss" "v2ray二进制文件下载失败，可能是地址失效或者网络异常！"
nvram set ss_enable=0
ssp_close
else
logger -t "ss" "v2ray二进制文件下载成功"
chmod -R 777 /tmp/v2ray
v2ray_enable=1
fi
	  if [ "$(nvram get v2_net_x$1)" == "kcp" ]; then
cat > "$v2_file" <<EOF
{
   "outbound": {
     "settings": {
       "vnext": [
         {
           "port": $(nvram get ssp_prot_x$1),
           "users": [
             {
               "id": "$(nvram get v2_vid_x$1)",
               "alterId": $(nvram get v2_aid_x$1),
               "security": "$(nvram get v2_security_x$1)"
             }
           ],
           "address": "$hostip"
         }
       ]
     },
     "mux": {
       "enabled": false
     },
     "protocol": "vmess",
     "streamSettings": {
       "network": "$(nvram get v2_net_x$1)",
       "tlsSettings": {
         "allowInsecure": false
       },
      "kcpSettings": {
         "readBufferSize": 2,
         "uplinkCapacity": 5,
         "header": {
           "type": "$(nvram get v2_type_mkcp_x$1)"
         },
         "mtu": 1350,
         "writeBufferSize": 2,
         "congestion": false,
         "downlinkCapacity": 20,
         "tti": 50
       },
       "security": "none"
     }
   },
   "log": {
     "loglevel": "warning"
   },
   "outboundDetour": [
     {
       "settings": {
         "keep": ""
       },
       "protocol": "freedom",
       "tag": "direct"
     }
   ],
   "inbound": {
     "sniffing": {
       "enabled": true,
       "destOverride": [
         "http",
         "tls"
       ]
     },
     "port": "$(nvram get ssp_local_port_x$1)",
     "protocol": "dokodemo-door",
     "settings": {
       "network": "tcp",
       "followRedirect": true
     }
   }
 }

EOF
fi
if [ "$(nvram get v2_net_x$1)" == "ws" ]; then
cat > "$v2_file" <<EOF
{
   "outbound": {
     "settings": {
       "vnext": [
         {
           "port": $(nvram get ssp_prot_x$1),
           "users": [
             {
               "id": "$(nvram get v2_vid_x$1)",
               "alterId": $(nvram get v2_aid_x$1),
               "security": "$(nvram get v2_security_x$1)"
             }
           ],
           "address": "$hostip"
         }
       ]
     },
     "mux": {
       "enabled": false
     },
     "protocol": "vmess",
     "streamSettings": {
       "network": "$(nvram get v2_net_x$1)",
       "tlsSettings": {
         "allowInsecure": false
       },
      "wsSettings": {
         "path": "$(nvram get v2_webs_path_x$1)",
         "header": {
           "Host": "$(nvram get v2_webs_host_x$1)"
         },
       "security": "none"
     }
   },
   "log": {
     "loglevel": "warning"
   },
   "outboundDetour": [
     {
       "settings": {
         "keep": ""
       },
       "protocol": "freedom",
       "tag": "direct"
     }
   ],
   "inbound": {
     "sniffing": {
       "enabled": true,
       "destOverride": [
         "http",
         "tls"
       ]
     },
     "port": "$(nvram get ssp_local_port_x$1)",
     "protocol": "dokodemo-door",
     "settings": {
       "network": "tcp",
       "followRedirect": true
     }
   }
 }

EOF
fi
if [ "$(nvram get v2_net_x$1)" == "h2" ]; then
cat > "$v2_file" <<EOF
{
   "outbound": {
     "settings": {
       "vnext": [
         {
           "port": $(nvram get ssp_prot_x$1),
           "users": [
             {
               "id": "$(nvram get v2_vid_x$1)",
               "alterId": $(nvram get v2_aid_x$1),
               "security": "$(nvram get v2_security_x$1)"
             }
           ],
           "address": "$hostip"
         }
       ]
     },
     "mux": {
       "enabled": false
     },
     "protocol": "vmess",
     "streamSettings": {
       "network": "$(nvram get v2_net_x$1)",
       "tlsSettings": {
         "allowInsecure": false
       },
      "httpSettings": {
         "path": "$(nvram get v2_h2_path_x$1)",
         "header": {
           "Host": "$(nvram get v2_h2_host_x$1)"
         },
       "security": "none"
     }
   },
   "log": {
     "loglevel": "warning"
   },
   "outboundDetour": [
     {
       "settings": {
         "keep": ""
       },
       "protocol": "freedom",
       "tag": "direct"
     }
   ],
   "inbound": {
     "sniffing": {
       "enabled": true,
       "destOverride": [
         "http",
         "tls"
       ]
     },
     "port": "$(nvram get ssp_local_port_x$1)",
     "protocol": "dokodemo-door",
     "settings": {
       "network": "tcp",
       "followRedirect": true
     }
   }
 }

EOF
fi
if [ "$(nvram get v2_net_x$1)" == "quic" ]; then
cat > "$v2_file" <<EOF
{
   "outbound": {
     "settings": {
       "vnext": [
         {
           "port": $(nvram get ssp_prot_x$1),
           "users": [
             {
               "id": "$(nvram get v2_vid_x$1)",
               "alterId": $(nvram get v2_aid_x$1),
               "security": "$(nvram get v2_security_x$1)"
             }
           ],
           "address": "$hostip"
         }
       ]
     },
     "mux": {
       "enabled": false
     },
     "protocol": "vmess",
     "streamSettings": {
       "network": "$(nvram get v2_net_x$1)",
       "tlsSettings": {
         "allowInsecure": false
       },
      "quicSettings": {
         "security": "$(nvram get v2_h2_path_x$1)",
         "key": {
           "type": $(nvram get v2_h2_host_x$1)
         },
       "security": "none"
     }
   },
   "log": {
     "loglevel": "warning"
   },
   "outboundDetour": [
     {
       "settings": {
         "keep": ""
       },
       "protocol": "freedom",
       "tag": "direct"
     }
   ],
   "inbound": {
     "sniffing": {
       "enabled": true,
       "destOverride": [
         "http",
         "tls"
       ]
     },
     "port": "$(nvram get ssp_local_port_x$1)",
     "protocol": "dokodemo-door",
     "settings": {
       "network": "tcp",
       "followRedirect": true
     }
   }
 }

EOF
fi
fi
}

get_arg_out() {
	case $ss_turn in
		1) echo "-o";;
		2) echo "-O";;
	esac
}

start_rules() {
logger -t "SS" "正在添加防火墙规则..."
	server=`nvram get ssp_server_x$1`
	cat /etc/storage/ss_ip.sh | grep -v '^!' | grep -v "^$" > /tmp/whileip.txt
	#resolve name
	if echo $server|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then         
	server=${server}
	elif  [ "$server" != "${server#*:[0-9a-fA-F]}" ] ;then
	server=${server}
	else
	server=`ping ${server} -s 1 -c 1 | grep PING | cut -d'(' -f 2 | cut -d')' -f1`
	 if echo $server|grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$">/dev/null; then
	  echo $server >/etc/storage/ssr_ip
	 else
	  server=`cat /etc/storage/ssr_ip`
	 fi
	fi
	
	local_port=$(nvram get ssp_local_port_x$1)
	lan_ac_ips=$lan_ac_ips
	lan_ac_mode="b"
	router_proxy="1"
	if [ "$UDP_RELAY_SERVER" != "nil" ]; then
	if [ "$GLOBAL_SERVER" = "$UDP_RELAY_SERVER" ]; then
		ARG_UDP="-u"
	elif [ -n "$UDP_RELAY_SERVER" ]; then
		ARG_UDP="-U"
		udp_server=`nvram get ssp_server_x$UDP_RELAY_SERVER`
		udp_local_port=`nvram get ssp_local_port_x$UDP_RELAY_SERVER`
	fi
	fi
	if [ -n "$lan_ac_ips" ]; then
		case "$lan_ac_mode" in
			w|W|b|B) ac_ips="$lan_ac_mode$lan_ac_ips";;
		esac
	fi
	#ac_ips="b"
#deal	gfw firewall rule
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
	echo "create china hash:net family inet hashsize 1024 maxelem 65536" > /tmp/china.ipset
awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/storage/chinadns/chnroute.txt >> /tmp/china.ipset
ipset -! flush china
ipset -! restore < /tmp/china.ipset 2>/dev/null
rm -f /tmp/china.ipset

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
		$(get_arg_out) $gfwmode $ARG_UDP
			
	return $?
}

start_pdnsd() {
	usr_dns="$1"
    usr_port="$2"

	tcp_dns_list="208.67.222.222, 208.67.220.220"
	[ -z "$usr_dns" ] && usr_dns="8.8.4.4"
	[ -z "$usr_port" ] && usr_port="53"
 dnsd_enable=`nvram get pdnsd_enable`
 echo $dnsd_enable
if [ $dnsd_enable = 0 ]; then
   if [ ! -d /tmp/pdnsd ];then
       mkdir -p /tmp/pdnsd
       echo -ne "pd13\000\000\000\000" >/tmp/pdnsd/pdnsd.cache
       chown -R nobody:nogroup /tmp/pdnsd
   fi
	cat > /tmp/pdnsd.conf <<EOF
global {
perm_cache=2048;
cache_dir="/tmp/pdnsd";
pid_file = /tmp/pdnsd.pid;
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
	label= "ssr-usrdns";
	ip = $usr_dns;
	port = $usr_port;
	timeout=6;
	uptest=none;
	interval=10m;
	purge_cache=off;
}
server {
	label= "ssr-pdnsd";
	ip = $tcp_dns_list;
	port = 443;
	timeout=6;
	uptest=none;
	interval=10m;
	purge_cache=off;
}
EOF

chmod 600 /tmp/pdnsd.conf
logger -t "SS" "正在启动pdnsd..."
/usr/bin/pdnsd -c /tmp/pdnsd.conf -d
fi
}


start_redir() {
logger -t "SS" "正在启动SS程序..."
	ARG_OTA=""
  gen_config_file $1 0
  stype=`nvram get ssp_type_x$1`
    if [ "$stype" == "ss" ] ;then
        sscmd="ss-redir"
       elif [ "$stype" == "ssr" ] ;then
        sscmd="ssr-redir"
       elif [ "$stype" == "v2ray" ] ;then
        sscmd="/tmp/v2ray"
    fi
	UDP_RELAY_SERVER=$(nvram get udp_relay_server)
	utype=`nvram get ssp_type_x$UDP_RELAY_SERVER`
    if [ "$utype" == "ss" ] ;then
        ucmd="ss-redir"
       elif [ "$utype" == "ssr" ] ;then
        ucmd="ssr-redir"
       elif [ "$utype" == "v2ray" ] ;then
        ucmd="/tmp/v2ray"
     fi
 
  #if [ "$(nvram get ss_threads)" = "0" ] ;then
  #  threads=$(cat /proc/cpuinfo | grep 'processor' | wc -l)
 # else
 #   threads=$(nvram get ss_threads)
 # fi
	if [ "$stype" == "ss" -o "$stype" == "ssr" ] ;then
    last_config_file=$CONFIG_FILE
     pid_file="/tmp/ssr-retcp.pid"
    for i in $(seq 1 $threads)  
    do 
      $sscmd -c $CONFIG_FILE $ARG_OTA -f /tmp/ssr-retcp_$i.pid >/dev/null 2>&1
    done
	redir_tcp=1
    echo "$(date "+%Y-%m-%d %H:%M:%S") Shadowsocks/ShadowsocksR $threads 线程启动成功!" >> /tmp/ssrplus.log  
  elif [ "$stype" == "v2ray" ] ;then
	$sscmd -config $v2_json_file >/dev/null 2>&1 &
    echo "$(date "+%Y-%m-%d %H:%M:%S") $($sscmd -version | head -1) 启动成功!" >> /tmp/ssrplus.log
	fi
#UDP模式完善中------->
	if [ "$UDP_RELAY_SERVER" != "nil" ] ;then
    redir_udp=1
    if [ "$utype" == "ss" -o "$utype" == "ssr" ] ;then
     # case "$(uci_get_by_name $UDP_RELAY_SERVER auth_enable)" in
     #   1|on|true|yes|enabled) ARG_OTA="-A";;
     #   *) ARG_OTA="";;
     # esac		
      gen_config_file $UDP_RELAY_SERVER 1
      last_config_file=$CONFIG_UDP_FILE
      pid_file="/tmp/ssr-reudp.pid"
      $ucmd -c $last_config_file $ARG_OTA -U -f /tmp/ssr-reudp.pid >/dev/null 2>&1
    elif [ "$utype" == "v2ray" ] ; then
        sed -i 's/"network": "tcp"/"network": "udp"/g' $v2_file
        $ucmd -config $v2udp_json_file >/dev/null 2>&1 &   
    fi
   fi
#UDP模式结束
	#deal with dns
	#start_pdnsd $dnsserver $dnsport
	if [ $(nvram get pdnsd_enable) = 0 ]; then
	/usr/bin/pdnsd.sh start $dnsserver
    pdnsd_enable_flag=1
	fi
	ss_switch=`nvram get switch_enable_x$1`
	if [ $ss_turn = "1" ] ;then
		if [ $ss_switch = "1" ] ;then
			if [ -z "$switch_server" ] ;then
				switch_time=$(nvram get ss_turn_s)
				switch_timeout=$(nvram get ss_turn_ss)
			/usr/bin/ssr-switch start $switch_time $switch_timeout &
				#switch_enable=1
			fi
		fi
	fi
	#add_cron 

	return $?
}

start_local() {
	local_server=$(nvram get socks5_proxy)
	[ "$local_server" = "nil" ] && return 1
	mkdir -p /var/run /var/etc
	gen_config_file $local_server 2
	/usr/bin/ssr-local -c $CONFIG_SOCK5_FILE -u  \
		-l $(nvram get socks5_proxy_prot) \
		-b 0.0.0.0 \
		-f /tmp/ssr-local.pid >/dev/null 2>&1
	local_enable=1	
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

ssp_start() { 
	GLOBAL_SERVER=`nvram get global_server`
	echo $GLOBAL_SERVER
	ss_enable=`nvram get ss_enable`
if [ $ss_enable != "0" ] && [ $GLOBAL_SERVER != "nil" ]; then
	start_redir $GLOBAL_SERVER
	start_rules $GLOBAL_SERVER
    dnsstr="$(nvram get tunnel_forward)"
    dnsserver=`echo "$dnsstr"|awk -F ':'  '{print $1}'`
    dnsport=`echo "$dnsstr"|awk -F ':'  '{print $2}'`
	if [ "$run_mode" = "gfw" ] ;then
	ipset add gfwlist $dnsserver 2>/dev/null
	cat /etc/storage/ss_dom.sh | grep -v '^!' | grep -v "^$" > /tmp/ss_dom.txt
	awk '{printf("server=/%s/127.0.0.1#5353\nipset=/%s/gfwlist\n", $1, $1 )}' /tmp/ss_dom.txt > /etc/storage/gfwlist/m.gfwlist.conf
	rm -f /tmp/ss_dom.txt
	sed -i '/gfwlist/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
conf-dir=/etc/storage/gfwlist/
EOF
  elif [ "$run_mode" = "oversea" ] ;then
  ipset add gfwlist $dnsserver 2>/dev/null
  mkdir -p /etc/storage/dnsmasq.oversea
  	sed -i '/dnsmasq-ss/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
conf-dir=/etc/storage/dnsmasq.oversea
EOF
else
ipset -N ss_spec_wan_ac hash:net 2>/dev/null
        ipset add ss_spec_wan_ac $dnsserver 2>/dev/null 
	fi
/sbin/restart_dhcpd
	start_local
	
if [ $(nvram get ss_watchcat) = 1 ] ;then
	let total_count=server_count+redir_tcp+redir_udp+tunnel_enable+v2ray_enable+local_enable+pdnsd_enable_flag+switch_enable
    if [ $total_count -gt 0 ]
    then
    #param:server(count) redir_tcp(0:no,1:yes)  redir_udp tunnel kcp local gfw
    /usr/bin/ssr-monitor $server_count $redir_tcp $redir_udp $tunnel_enable $v2ray_enable $local_enable $pdnsd_enable_flag $switch_enable >/dev/null 2>&1 &
    fi
	fi
	
	ENABLE_SERVER=$(nvram get global_server)
	[ "$ENABLE_SERVER" = "-1" ] && return 1
	logger -t "SS" "启动成功。"
	fi
}

ssp_close() {
	/usr/bin/ss-rules -f
	/usr/bin/pdnsd.sh stop
	srulecount=`iptables -L|grep SSR-SERVER-RULE|wc -l`
	if [ $srulecount -gt 0 ] ;then
	iptables -F SSR-SERVER-RULE
	iptables -t filter -D INPUT  -j SSR-SERVER-RULE
	iptables -X SSR-SERVER-RULE 2>/dev/null
	fi
    kill -9 $(ps | grep ssr-switch | grep -v grep | awk '{print $1}') >/dev/null 2>&1
    kill -9 $(ps | grep ssr-monitor | grep -v grep | awk '{print $1}') >/dev/null 2>&1	
	killall -q -9 ss-redir
	killall -q -9 ssr-redir
	killall -q -9 v2ray
	killall -q -9 ssr-server
	killall -q -9 ssr-local
	
	cat /tmp/pdnsd.pid | xargs kill -9

	sed -i '/gfwlist/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
	if [ -f "/etc/storage/dnsmasq-ss.d" ]; then
		rm -f /etc/storage/dnsmasq-ss.d
		
  fi 
  /sbin/restart_dhcpd
}

ressp() {
	#if [ -z "$switch_server" ] ;then
	GLOBAL_SERVER=`nvram get global_server`
	echo $GLOBAL_SERVER
	ss_enable=`nvram get ss_enable`
	#else
	#GLOBAL_SERVER=$switch_server
	#switch_enable=1
	#fi
	#if rules ;then
if [ $ss_enable != "0" ] && [ $GLOBAL_SERVER != "nil" ]; then
    start_redir $1
	start_rules $1
	#start_rules $GLOBAL_SERVER
  
  
	if [ "$run_mode" = "gfw" ] ;then
	#mkdir -p /etc/storage/dnsmasq-ss.d
		cat /etc/storage/ss_dom.sh | grep -v '^!' | grep -v "^$" > /tmp/ss_dom.txt
	awk '{printf("server=/%s/127.0.0.1#5353\nipset=/%s/gfwlist\n", $1, $1 )}' /tmp/ss_dom.txt > /etc/storage/gfwlist/m.gfwlist.conf
	rm -f /tmp/ss_dom.txt
	sed -i '/gfwlist/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
conf-dir=/etc/storage/gfwlist/
EOF
  elif [ "$run_mode" = "oversea" ] ;then
  mkdir -p /etc/storage/dnsmasq.oversea
  	sed -i '/dnsmasq-ss/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
conf-dir=/etc/storage/dnsmasq.oversea
EOF

	fi
/sbin/restart_dhcpd
	start_local
	
if [ $(nvram get ss_watchcat) = 1 ] ;then
	let total_count=server_count+redir_tcp+redir_udp+tunnel_enable+v2ray_enable+local_enable+pdnsd_enable_flag+switch_enable
    if [ $total_count -gt 0 ]
    then
    #param:server(count) redir_tcp(0:no,1:yes)  redir_udp tunnel kcp local gfw
    /usr/bin/ssr-monitor $server_count $redir_tcp $redir_udp $tunnel_enable $v2ray_enable $local_enable $pdnsd_enable_flag $switch_enable >/dev/null 2>&1 &
    fi
	fi
	
	ENABLE_SERVER=$(nvram get global_server)
	[ "$ENABLE_SERVER" = "-1" ] && return 1
	logger -t "SS" "启动成功。"
	fi
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
   ressp $2
   ;;
*)
	echo "check"
	#exit 0
	;;
esac
