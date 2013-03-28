#!/bin/sh

dir_storage="/etc/storage"

func_start()
{
	key_s=""
	key_x="-x"
	key_4=""
	
	[ ! -d "$dir_storage" ] && mkdir -p $dir_storage
	
	if [ ! -f "$dir_storage/dropbear_rsa_host_key" ] ; then
		/usr/bin/dropbearkey -t rsa -f "$dir_storage/dropbear_rsa_host_key"
	fi
	
	if [ ! -f "$dir_storage/dropbear_dss_host_key" ] ; then
		/usr/bin/dropbearkey -t dss -f "$dir_storage/dropbear_dss_host_key"
	fi
	
	if [ -n "$1" ] ; then
		key_s="-s"
	fi
	
	db_kex_new=`nvram get db_kex_new`
	if [ "$db_kex_new" == "1" ] ; then
		key_x=""
	fi
	
	ip6_service=`nvram get ip6_service`
	if [ -z "$ip6_service" ] && [ -d /proc/sys/net/ipv6 ] ; then
		key_4="-4"
	fi
	
	/usr/sbin/dropbear $key_x $key_4 $key_s
}

func_stop()
{
	killall -q dropbear
}

func_newkeys()
{
	rm -f "$dir_storage/dropbear_rsa_host_key"
	rm -f "$dir_storage/dropbear_dss_host_key"
	
	[ ! -d "$dir_storage" ] && mkdir -p $dir_storage
	/usr/bin/dropbearkey -t rsa -f "$dir_storage/dropbear_rsa_host_key"
	/usr/bin/dropbearkey -t dss -f "$dir_storage/dropbear_dss_host_key"
}

case "$1" in
start)
	func_start $2
	;;
stop)
	func_stop
	;;
newkeys)
	func_newkeys
	;;
*)
	echo "Usage: $0 {start|stop|newkeys}"
	exit 1
	;;
esac

exit 0
