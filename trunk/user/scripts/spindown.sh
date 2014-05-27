#!/bin/sh

[ ! -x /sbin/hdparm ] && exit 1

# check param
[ -z "$1" ] && exit 1

# check device is mounted
grep -q /dev/$1 /proc/mounts && exit 1

if [ -r /sys/block/$1/removable ] ; then
	dev_removable=`cat /sys/block/$1/removable`
	[ $dev_removable -eq 0 ] && /sbin/hdparm -y /dev/$1
fi

