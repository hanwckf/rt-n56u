#!/bin/sh
dns_enable=`nvram get dns_enable`
dns_server=`nvram get dns_server`
dns_server_bind=`nvram get dns_server_bind`
dns_server_port=`nvram get dns_server_port`
PDNSD_BIN="/usr/bin/pdnsd"
ss_dns=$2
pdnsd_genconfig() {	
  if [ ! -f /tmp/dnscache/pdnsd.cache ]; then
    mkdir -p /tmp/dnscache
    echo -ne "pd13\000\000\000\000" > /tmp/dnscache/pdnsd.cache
    chown -R nobody.nogroup /tmp/dnscache
	fi
if [ ! -z $ss_dns ];then
dns_server="$dns_server,$ss_dns"
fi
	cat > /tmp/dnscache.conf <<EOF
global {
    perm_cache=512;        # dns缓存大小，单位KB，建议不要写的太大
    cache_dir="/tmp/dnscache";     # 缓存文件的位置
    pid_file = /tmp/dnscache.pid;
    server_ip = $dns_server_bind;        # pdnsd监听的网卡，0.0.0.0是全部网卡
    server_port=$dns_server_port;           # pdnsd监听的端口，不要和别的服务冲突即可
    status_ctl = on;
    paranoid=on;                  # 二次请求模式，如果请求主DNS服务器返回的是垃圾地址，就向备用服务器请求
    query_method=tcp_udp;      
    neg_domain_pol = off;  
    par_queries = 400;          # 最多同时请求数
    min_ttl = 1h;               # DNS结果最短缓存时间
    max_ttl = 1w;               # DNS结果最长缓存时间
    timeout = 10;               # DNS请求超时时间，单位秒
}

server {  
    label = "routine";         
    ip = $dns_server;     # 这里为主要上级 dns 的 ip 地址，建议填写一个当地最快的DNS地址  
    timeout = 5;              # DNS请求超时时间
    reject = 74.125.127.102,  # 以下是脏IP，也就是DNS污染一般会返回的结果，如果收到如下DNS结果会触发二次请求（TCP协议一般不会碰到脏IP）
        74.125.155.102,  
        74.125.39.102,  
        74.125.39.113,  
        209.85.229.138,  
        128.121.126.139,  
        159.106.121.75,  
        169.132.13.103,  
        192.67.198.6,  
        202.106.1.2,  
        202.181.7.85,  
        203.161.230.171,  
        203.98.7.65,  
        207.12.88.98,  
        208.56.31.43,  
        209.145.54.50,  
        209.220.30.174,  
        209.36.73.33,  
        211.94.66.147,  
        213.169.251.35,  
        216.221.188.182,  
        216.234.179.13,  
        243.185.187.39,  
        37.61.54.158,  
        4.36.66.178,  
        46.82.174.68,  
        59.24.3.173,  
        64.33.88.161,  
        64.33.99.47,  
        64.66.163.251,  
        65.104.202.252,  
        65.160.219.113,  
        66.45.252.237,  
        69.55.52.253,  
        72.14.205.104,  
        72.14.205.99,  
        78.16.49.15,  
        8.7.198.45,  
        93.46.8.89,  
        37.61.54.158,  
        243.185.187.39,  
        190.93.247.4,  
        190.93.246.4,  
        190.93.245.4,  
        190.93.244.4,  
        65.49.2.178,  
        189.163.17.5,  
        23.89.5.60,  
        49.2.123.56,  
        54.76.135.1,  
        77.4.7.92,  
        118.5.49.6,  
        159.24.3.173,  
        188.5.4.96,  
        197.4.4.12,  
        220.250.64.24,  
        243.185.187.30,  
        249.129.46.48,  
        253.157.14.165;  
    reject_policy = fail;  
}

server {  
    label = "special";                  # 这个随便写  
    ip = 208.67.222.222,208.67.220.220; # 这里为备用DNS服务器的 ip 地址  
    port = 53;                        # 推荐使用53以外的端口（DNS服务器必须支持） 
    proxy_only = on;
    timeout = 5;  
}  

source {
	owner=localhost;
//	serve_aliases=on;
	file="/etc/hosts";
}

rr {
	name=localhost;
	reverse=on;
	a=127.0.0.1;
	owner=localhost;
	soa=localhost,root.localhost,42,86400,900,86400,86400;
}
EOF

chmod -R 744 /tmp/dnscache.conf
	echo "Start DNS Cache"
	logger -t "pdnsd" "启动DNS加速。"
}

dns_close() {
  cat /tmp/dnscache.pid | xargs kill -9
  revert_dns
  echo "Stop DNS Cache"
  logger -t "pdnsd" "关闭DNS加速。"
}

change_dns() {
sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
no-resolv
server=127.0.0.1#$dns_server_port
EOF

/sbin/restart_dhcpd
}

revert_dns() {
sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
/sbin/restart_dhcpd
}

dns_start(){
 # if [ $dns_enable -eq 1 ];  then
  pdnsd_genconfig
  if [ ! -f "/var/pdnsddns" ]; then
  ln -sf /usr/bin/pdnsd /var/pdnsddns
  fi
/var/pdnsddns -c /tmp/dnscache.conf -d
    change_dns
    
 #fi
}

case $1 in
start)
	dns_start
	;;
stop)
	dns_close
	;;
*)
	echo "check"
	;;
esac
