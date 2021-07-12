#!/bin/sh
#from aaron
killall npc
mkdir -p /tmp/npc
tmpconf="/tmp/npc/npc.conf"
LOGFILE="/tmp/npc.log"

if [ -f $tmpconf ] ; then 
	rm $tmpconf
fi

npc_enable=`nvram get npc_enable`
server_addr=`nvram get npc_server_addr`
server_port=`nvram get npc_server_port`
protocol=`nvram get npc_protocol`
vkey=`nvram get npc_vkey`
compress=`nvram get npc_compress`
crypt=`nvram get npc_crypt`
Log_level=`nvram get npc_log_level`

echo "[common]" >$tmpconf
echo "server_addr=$server_addr:$server_port" >>$tmpconf
echo "conn_type=$protocol" >>$tmpconf
echo "vkey=$vkey" >>$tmpconf
echo "auto_reconnection=true" >>$tmpconf

if [ "$compress" = "1" ] ; then
	echo "compress=true" >>$tmpconf
else
	echo "compress=false" >>$tmpconf
fi

if [ "$crypt" = "1" ] ; then
	echo "crypt=true" >>$tmpconf
else
	echo "crypt=false" >>$tmpconf
fi

if [ "$npc_enable" = "1" ] ; then
	npc_bin="/usr/bin/npc"
	if [ ! -f "$npc_bin" ]; then
		if [ ! -f "/tmp/npc/npc" ];then
			wget -c -P /tmp/npc https://github.com/etion2008/aaron/raw/main/npc/npc
			if [ ! -f "/tmp/npc/npc" ]; then
				logger -t "NPC" "npc二进制文件下载失败，可能是地址失效或者网络异常！"
				nvram set npc_enable=0
				npc_close
			else
				logger -t "NPC" "npc二进制文件下载成功"
				chmod -R 777 /tmp/npc/npc
				npc_bin="/tmp/npc/npc"
			fi
		else
			npc_bin="/tmp/npc/npc"
		fi
	fi

	$npc_bin -config=$tmpconf -log_level=$Log_level -log_path=$LOGFILE -debug=false 2>&1 &
fi
