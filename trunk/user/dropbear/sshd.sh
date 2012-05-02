#!/bin/sh

dir_storage="/etc/storage"

func_start()
{
	[ ! -d "$dir_storage" ] && mkdir -p $dir_storage
	
	if [ ! -f "$dir_storage/dropbear_rsa_host_key" ] ; then
		/usr/bin/dropbearkey -t rsa -f "$dir_storage/dropbear_rsa_host_key"
	fi
	
	if [ ! -f "$dir_storage/dropbear_dss_host_key" ] ; then
		/usr/bin/dropbearkey -t dss -f "$dir_storage/dropbear_dss_host_key"
	fi
	
	if [ -n "$1" ] ; then
		/usr/sbin/dropbear -s -W 65536
	else
		/usr/sbin/dropbear -W 65536
	fi
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
