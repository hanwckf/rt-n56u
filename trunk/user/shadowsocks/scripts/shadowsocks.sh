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
v2_json_file="/tmp/v2-redir.json"
v2udp_json_file="/tmp/v2-udpredir.json"
server_count=0
redir_tcp=0
v2ray_enable=0
redir_udp=0
tunnel_enable=0
local_enable=0
pdnsd_enable_flag=0
threads=1
wan_bp_ips="/tmp/whiteip.txt"
wan_fw_ips="/tmp/blackip.txt"
lan_fp_ips="/tmp/lan_ip.txt"
run_mode=`nvram get ss_run_mode`
ss_turn=`nvram get ss_turn`
ss_udp=`nvram get ss_udp`
lan_con=`nvram get lan_con`
gen_config_file() {
hostip=`nvram get ssp_server_x$1`
if [ $2 = "0" ] ;then
config_file=$CONFIG_FILE
elif [ $2 = "1" ]; then
config_file=$CONFIG_UDP_FILE
else
config_file=$CONFIG_SOCK5_FILE
fi
fastopen="false";
stype=`nvram get ssp_type_x$1`
logger -t "SS" "正在创建$stype客户端的json文件..."
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
	"plugin": "$(nvram get ss_plugin_x$1)"
}
EOF
elif [ "$stype" == "ssr" ] ;then
cat <<-EOF >$config_file
{
	"server": "$hostip",
	"server_port": $(nvram get ssp_prot_x$1),
	"local_address": "0.0.0.0",
	"local_port": $(nvram get ssp_local_port_x$1),
	"password": "$(nvram get ss_key_x$1)",
	"timeout": 60,
	"method": "$(nvram get ss_method_x$1)",
	"protocol": "$(nvram get ss_protocol_x$1)",
	"protocol_param": "$(nvram get ss_proto_param_x$1)",
	"obfs": "$(nvram get ss_obfs_x$1)",
	"obfs_param": "$(nvram get ss_obfs_param_x$1)"
}
EOF
elif [ "$stype" == "v2ray" ] ;then
v2_file=$v2_json_file
curl -k -s -o /tmp/v2ray --connect-timeout 10 --retry 3 https://dev.tencent.com/u/dtid_39de1afb676d0d78/p/kp/git/raw/master/v2ray
if [ ! -f "/tmp/v2ray" ]; then
logger -t "SS" "v2ray二进制文件下载失败，可能是地址失效或者网络异常！"
nvram set ss_enable=0
ssp_close
else
logger -t "SS" "v2ray二进制文件下载成功"
chmod -R 777 /tmp/v2ray
v2ray_enable=1
fi
#创建v2ray json文件的代码用的是hiboyhiboy的,特此感谢
vmess_link_add=$hostip
vmess_link_port=$(nvram get ssp_prot_x$1)
vmess_link_id=$(nvram get v2_vid_x$1)
vmess_link_aid=$(nvram get v2_aid_x$1)
vmess_link_net=$(nvram get v2_net_x$1)
if [ $(nvram get v2_tls_x$1) = "1" ];then
vmess_link_tls="tls"
else
vmess_link_tls="none"
fi
mk_vmess=$(json_int_vmess_settings)
mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"address"];"'$vmess_link_add'")')
mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"users",0,"alterId"];'$vmess_link_aid')')
mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"users",0,"id"];"'$vmess_link_id'")')
mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["vnext",0,"port"];'$vmess_link_port')')
vmess_settings=$mk_vmess
mk_vmess=$(json_int_vmess_streamSettings)
mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["network"];"'$vmess_link_net'")')
mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["security"];"'$vmess_link_tls'")')
# tcp star
if [ "$vmess_link_net" = "tcp" ] ; then
[ ! -z "$(nvram get v2_type_tcp_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["tcpSettings","type"];"'$(nvram get v2_type_tcp_x$1)'")')
fi
# tcp end
# kcp star
if [ "$vmess_link_net" = "kcp" ] ; then
[ ! -z "$(nvram get v2_type_mkcp_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["kcpSettings","header","type"];"'$(nvram get v2_type_mkcp_x$1)'")')
fi
# kcp end
# ws star
if [ "$vmess_link_net" = "ws" ] ; then
[ ! -z "$(nvram get v2_webs_path_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["wsSettings","path"];"'$(nvram get v2_webs_path_x$1)'")')
[ ! -z "$(nvram get v2_webs_host_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["wsSettings","headers","Host"];"'$(nvram get v2_webs_host_x$1)'")')
fi
# ws end
# h2 star
if [ "$vmess_link_net" = "h2" ] ; then
[ ! -z "$(nvram get v2_http2_path_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["httpSettings","path"];"'$(nvram get v2_http2_path_x$1)'")')
vmess_link_host=$(echo $(nvram get v2_http2_host_x$1) | sed 's/,/ /g')
link_host_i=0
for link_host in $vmess_link_host
do
mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["httpSettings","host",'$link_host_i'];"'$link_host'")')
link_host_i=$(( link_host_i + 1 ))
done
fi
# h2 end
# quic star
if [ "$vmess_link_net" = "quic" ] ; then
[ ! -z "$(nvram get v2_quic_header_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["quicSettings","header","type"];"'$(nvram get v2_quic_header_x$1)'")')
[ ! -z "$(nvram get v2_quic_security_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["quicSettings","security"];"'$(nvram get v2_quic_security_x$1)'")')
[ ! -z "$(nvram get v2_quic_key_x$1)" ] && mk_vmess=$(echo $mk_vmess | jq --raw-output 'setpath(["quicSettings","key"];"'$(nvram get v2_quic_key_x$1)'")')
fi
# quic end
vmess_streamSettings=$mk_vmess
mk_vmess=$(json_int)
mk_vmess=$(echo $mk_vmess| jq --raw-output 'setpath(["outbounds",0,"settings"];'"$vmess_settings"')')
mk_vmess=$(echo $mk_vmess| jq --raw-output 'setpath(["outbounds",0,"streamSettings"];'"$vmess_streamSettings"')')
mk_vmess=$(echo $mk_vmess| jq --raw-output 'setpath(["outbounds",0,"protocol"];"vmess")')
if [ $ss_udp = "1" ];then
mk_vmess=$(echo $mk_vmess| jq --raw-output 'setpath(["inbounds",0,"settings","udp"];'true')')
fi
echo $mk_vmess| jq --raw-output '.' > $v2_file
#创建json文件结束
fi
}
start_rules() {
logger -t "SS" "正在添加防火墙规则..."
server=`nvram get ssp_server_x$1`
cat /etc/storage/ss_ip.sh | grep -v '^!' | grep -v "^$" > $wan_fw_ips
cat /etc/storage/ss_wan_ip.sh | grep -v '^!' | grep -v "^$" > $wan_bp_ips
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
if [ "$ss_udp" = 1 ]; then
ARG_UDP="-u"
fi
if [ -n "$lan_ac_ips" ]; then
case "$lan_ac_mode" in
	w|W|b|B) ac_ips="$lan_ac_mode$lan_ac_ips";;
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
if [ "$lan_con" = "0" ];then
rm -f $lan_fp_ips
lancon="all"
lancons="全部IP走代理"
cat /etc/storage/ss_lan_ip.sh | grep -v '^!' | grep -v "^$" > $lan_fp_ips
elif [ "$lan_con" = "1" ];then
rm -f $lan_fp_ips
lancon="bip"
lancons="指定IP走代理,请到规则管理页面添加需要走代理的IP。"
cat /etc/storage/ss_lan_bip.sh | grep -v '^!' | grep -v "^$" > $lan_fp_ips
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
start_pdnsd() 
{
pdnsd_bin="/usr/bin/pdnsd"
pdnsd_cache="/tmp/pdnsd"
pdnsd_file="/tmp/pdnsd.conf"
pdnsd_pid="/tmp/pdnsd.pid"
usr_dns="$1"
usr_port="$2"
tcp_dns_list="208.67.222.222, 208.67.220.220"
[ -z "$usr_dns" ] && usr_dns="8.8.4.4"
[ -z "$usr_port" ] && usr_port="53"
dnsd_enable=`nvram get pdnsd_enable`
if [ $dnsd_enable = 0 ]; then
if [ ! -d $pdnsd_cache ];then
	mkdir -p $pdnsd_cache
	echo -ne "pd13\000\000\000\000" >$pdnsd_cache/pdnsd.cache
	chown -R nobody:nogroup $pdnsd_cache
fi
cat > $pdnsd_file <<EOF
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
ss_switch=`nvram get backup_server`
if [ $ss_switch != "nil" ] ;then
switch_time=$(nvram get ss_turn_s)
switch_timeout=$(nvram get ss_turn_ss)
/usr/bin/ssr-switch start $switch_time $switch_timeout &
socks="-o"
fi
return $?
}
start_dns()
{
dnsstr="$(nvram get tunnel_forward)"
dnsserver=`echo "$dnsstr"|awk -F ':'  '{print $1}'`
dnsport=`echo "$dnsstr"|awk -F ':'  '{print $2}'`
if [ $(nvram get pdnsd_enable) = 0 ]; then
start_pdnsd $dnsserver $dnsport	
pdnsd_enable_flag=1
fi
if [ "$run_mode" = "router" ]; then
echo "create china hash:net family inet hashsize 1024 maxelem 65536" > /tmp/china.ipset
awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/storage/chinadns/chnroute.txt >> /tmp/china.ipset
ipset -! flush china
ipset -! restore < /tmp/china.ipset 2>/dev/null
rm -f /tmp/china.ipset
elif [ "$run_mode" = "gfw" ] ;then
ipset add gfwlist $dnsserver 2>/dev/null
logger -st "SS" "开始处理gfwlist..."
rm -rf /etc/storage/gfwlist
mkdir -p /etc/storage/gfwlist/
cat /etc/storage/ss_dom.sh | grep -v '^!' | grep -v "^$" > /tmp/ss_dom.txt
if [ $(nvram get pdnsd_enable) = 0 ]; then
awk '{printf("server=/%s/127.0.0.1#5353\nipset=/%s/gfwlist\n", $1, $1 )}' /etc_ro/gfwlist_list.conf > /etc/storage/gfwlist/gfwlist_list.conf
awk '{printf("server=/%s/127.0.0.1#5353\nipset=/%s/gfwlist\n", $1, $1 )}' /tmp/ss_dom.txt > /etc/storage/gfwlist/m.gfwlist.conf
else
awk '{printf("ipset=/%s/gfwlist\n", $1, $1 )}' /etc_ro/gfwlist_list.conf > /etc/storage/gfwlist/gfwlist_list.conf
awk '{printf("ipset=/%s/gfwlist\n", $1, $1 )}' /tmp/ss_dom.txt > /etc/storage/gfwlist/m.gfwlist.conf
fi
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
start_watchcat()
{
if [ $(nvram get ss_watchcat) = 1 ] ;then
let total_count=server_count+redir_tcp+redir_udp+tunnel_enable+v2ray_enable+local_enable+pdnsd_enable_flag
if [ $total_count -gt 0 ]
then
#param:server(count) redir_tcp(0:no,1:yes)  redir_udp tunnel kcp local gfw
/usr/bin/ssr-monitor $server_count $redir_tcp $redir_udp $tunnel_enable $v2ray_enable $local_enable $pdnsd_enable_flag >/dev/null 2>&1 &
fi
fi
}
auto_update(){
sed -i '/update_chnroute/d' /etc/storage/cron/crontabs/$http_username
sed -i '/update_gfwlist/d' /etc/storage/cron/crontabs/$http_username
sed -i '/ss-watchcat/d' /etc/storage/cron/crontabs/$http_username
if [ $(nvram get ss_update_chnroute) = "1" ]; then
cat >> /etc/storage/cron/crontabs/$http_username << EOF
0 8 */10 * * /usr/bin/update_chnroute.sh > /dev/null 2>&1
EOF
fi
if [ $(nvram get ss_update_gfwlist) = "1" ]; then
cat >> /etc/storage/cron/crontabs/$http_username << EOF
0 7 */10 * * /usr/bin/update_gfwlist.sh > /dev/null 2>&1
EOF
fi
}
ssp_close() {
/usr/bin/ss-rules -f
kill -9 $(ps | grep ssr-switch | grep -v grep | awk '{print $1}') >/dev/null 2>&1
kill -9 $(ps | grep ssr-monitor | grep -v grep | awk '{print $1}') >/dev/null 2>&1	
killall -q -9 ss-redir
killall -q -9 ssr-redir
killall -q -9 v2ray
killall -q -9 ssr-server
killall -q -9 ssr-local
killall -9 pdnsd
sed -i '/gfwlist/d' /etc/storage/dnsmasq/dnsmasq.conf
sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
if [ -f "/etc/storage/dnsmasq-ss.d" ]; then
rm -f /etc/storage/dnsmasq-ss.d
fi 
/sbin/restart_dhcpd
}
rssp_close() {
/usr/bin/ss-rules -f
kill -9 $(ps | grep ssr-monitor | grep -v grep | awk '{print $1}') >/dev/null 2>&1	
killall -q -9 ss-redir
killall -q -9 ssr-redir
killall -q -9 v2ray
killall -q -9 ssr-server
killall -q -9 ssr-local
killall -9 pdnsd
sed -i '/gfwlist/d' /etc/storage/dnsmasq/dnsmasq.conf
sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
if [ -f "/etc/storage/dnsmasq-ss.d" ]; then
rm -f /etc/storage/dnsmasq-ss.d
fi 
/sbin/restart_dhcpd
}
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
"allowInsecure": true
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
"loglevel": "warning"
},
"inbounds": [
{
  "port": "1080",
  "listen": "0.0.0.0",
  "protocol": "dokodemo-door",
  "settings": {
	"network": "tcp,udp",
	"timeout": 30,
	"followRedirect": true,
	"udp": false
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
rssp_close
ressp
;;
*)
echo "check"
#exit 0
;;
esac