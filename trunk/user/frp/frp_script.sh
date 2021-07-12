#!/bin/sh
#from hiboy
killall frpc frps
mkdir -p /tmp/frp
#启动frp功能后会运行以下脚本
#frp项目地址教程: https://github.com/fatedier/frp/blob/master/README_zh.md
#请自行修改 token 用于对客户端连接进行身份验证
# IP查询： http://119.29.29.29/d?dn=github.com

cat > "/tmp/frp/myfrpc.ini" <<-\EOF
# ==========客户端配置：==========
[common]
server_addr = 211.10.10.1
server_port = 7000
token = 12345

#log_file = /tmp/frpc.log
#log_level = info
#log_max_days = 3

[web]
remote_port = 6000
type = http
local_ip = 192.168.2.1
local_port = 80
subdomain = test
#host_header_rewrite = 实际你内网访问的域名，可以供公网的域名不一致，如果一致可以不写
# ====================
EOF

#请手动配置【外部网络 (WAN) - 端口转发 (UPnP)】开启 WAN 外网端口
cat > "/tmp/frp/myfrps.ini" <<-\EOF
# ==========服务端配置：==========
[common]
bind_port = 7000
dashboard_port = 7500
# dashboard 用户名密码，默认都为 admin
dashboard_user = admin
dashboard_pwd = admin
vhost_http_port = 88
token = 12345
subdomain_host = frps.com
max_pool_count = 50
#log_file = /tmp/frps.log
#log_level = info
#log_max_days = 3
# ====================
EOF

#启动：
frpc_enable=`nvram get frpc_enable`
frps_enable=`nvram get frps_enable`

if [ "$frpc_enable" = "1" ] ; then
	frpc_bin="/usr/bin/frpc"
	if [ ! -f "$frpc_bin" ]; then
		if [ ! -f "/tmp/frp/frpc" ];then
			wget -c -P /tmp/frp https://github.com/etion2008/aaron/raw/main/frpc/frpc
			if [ ! -f "/tmp/frp/frpc" ]; then
				logger -t "FRPC" "frpc二进制文件下载失败，可能是地址失效或者网络异常！"
				nvram set frpc_enable=0
				frpc_close
			else
				logger -t "FRPC" "frpc二进制文件下载成功"
				chmod -R 777 /tmp/frp/frpc
				frpc_bin="/tmp/frp/frpc"
			fi
		else
			frpc_bin="/tmp/frp/frpc"
		fi
	fi

	$frpc_bin -c /tmp/frp/myfrpc.ini 2>&1 &
fi

if [ "$frps_enable" = "1" ] ; then
	frps_bin="/usr/bin/frps"
	if [ ! -f "$frps_bin" ]; then
		if [ ! -f "/tmp/frp/frps" ];then
			wget -c -P /tmp/frp https://github.com/etion2008/aaron/raw/main/frps/frps
			if [ ! -f "/tmp/frp/frps" ]; then
				logger -t "FRPS" "frps二进制文件下载失败，可能是地址失效或者网络异常！"
				nvram set frps_enable=0
				frps_close
			else
				logger -t "FRPS" "frps二进制文件下载成功"
				chmod -R 777 /tmp/frp/frps
				frps_bin="/tmp/frp/frps"
			fi
		else
			frps_bin="/tmp/frp/frps"
		fi
	fi

	$frps_bin -c /tmp/frp/myfrps.ini 2>&1 &
fi
 