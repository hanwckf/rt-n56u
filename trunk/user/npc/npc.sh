#!/bin/sh
npc_enable=`nvram get npc_enable`
http_username=`nvram get http_username`

check_npc () 
{
	check_net
	result_net=$?
	if [ "$result_net" = "1" ] ;then
		if [ -z "`pidof npc`" ] && [ "$npc_enable" = "1" ];then
			npc_start
		fi
	else

		logger -t "npc" "npc断线重连"
	fi
}

check_net() 
{
	/bin/ping -c 3 www.baidu.com -w 5 >/dev/null 2>&1
	if [ "$?" == "0" ]; then
		return 1
	else
		return 2
		logger -t "npc" "检测到互联网未能成功访问,稍后再尝试启动npc"
	fi
}

npc_start () 
{
	/etc/storage/npc_script.sh
	sed -i '/npc/d' /etc/storage/cron/crontabs/$http_username
	cat >> /etc/storage/cron/crontabs/$http_username << EOF
*/5 * * * * /bin/sh /usr/bin/npc.sh C >/dev/null 2>&1
EOF
	[ ! -z "`pidof npc`" ] && logger -t "npc" "npc启动成功"
}

npc_close () 
{
	if [ "$npc_enable" = "0" ]; then
		if [ ! -z "`pidof npc`" ]; then
		killall -9 npc npc_script.sh
		[ -z "`pidof npc`" ] && logger -t "npc" "已停止 npc"
	    fi
	fi

	if [ "$npc_enable" = "0" ]; then
	sed -i '/npc/d' /etc/storage/cron/crontabs/$http_username
	fi
}


case $1 in
start)
	npc_start
	;;
stop)
	npc_close
	;;
C)
	check_npc
	;;
esac
