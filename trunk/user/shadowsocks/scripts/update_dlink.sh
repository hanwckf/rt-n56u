#!/bin/sh
#===================================================================
#chongshengB 20200319


start_link() {
logger -t "SS" "开始运行订阅脚本...请稍后..."
grep -v '^#' /etc/storage/ss_dlink.sh | grep -v "^$" > /tmp/dlist.txt
dbus list ssconf_basic_json | cut -d '_' -f 4 | cut -d '=' -f 1 > /tmp/dlinkold.txt
lua /etc_ro/ss/dlink.lua
mtd_storage.sh save >/dev/null 2>&1
}

reset_link() {
	confs=`dbus list ssconf_basic_ | cut -d "=" -f 1`
	logger -t "SS" "关闭ShadowSocksR Plus+..."
	/usr/bin/shadowsocks.sh stop 
	logger -t "SS" "正在删除订阅节点..."
	for conf in $confs
	do
		dbus remove $conf
	done
	nvram set ss_enable=0
	nvram set global_server="nil"
	nvram set udp_relay_server="nil"
	logger -t "SS" "已重置订阅节点文件,请手动刷新页面..."
	mtd_storage.sh save >/dev/null 2>&1
}

update_link() {
logger -t "SS" "开始更新订阅脚本..."
grep -v '^#' /etc/storage/ss_dlink.sh | grep -v "^$" > /tmp/dlist.txt
dbus list ssconf_basic_json | cut -d '_' -f 4 | cut -d '=' -f 1 > /tmp/dlinkold.txt
lua /etc_ro/ss/dlink.lua
mtd_storage.sh save >/dev/null 2>&1
}

case $1 in
start)
	start_link
	;;
reset)
	reset_link
	;;
update)
	update_link
	;;
*) 
    ;;

esac
