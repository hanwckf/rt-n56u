#!/bin/sh
#===================================================================
#chongshengB 20200228


start_link() {
logger -t "SS" "开始运行订阅脚本...请稍后..."
grep -v '^#' /etc/storage/ss_dlink.sh | grep -v "^$" > /tmp/dlist.txt
dbus list ssconf_basic_json | cut -d '_' -f 4 | cut -d '=' -f 1 > /tmp/dlinkold.txt
lua /etc_ro/ss/dlink.lua
}

reset_link() {
	confs=`dbus list ssconf_basic_ | cut -d "=" -f 1`
	logger -t "SS" "正在删除订阅节点..."
	for conf in $confs
	do
		dbus remove $conf
	done
	logger -t "SS" "已重置订阅节点文件,请手动刷新页面..."
}

update_link() {
logger -t "SS" "开始更新订阅脚本..."
grep -v '^#' /etc/storage/ss_dlink.sh | grep -v "^$" > /tmp/dlist.txt
dbus list ssconf_basic_json | cut -d '_' -f 4 | cut -d '=' -f 1 > /tmp/dlinkold.txt
lua /etc_ro/ss/dlink.lua
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
