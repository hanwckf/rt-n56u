#!/bin/sh

[ ! -x /sbin/hdparm ] && exit 1

hdd_spindt=`nvram get hdd_spindt`
[ -z "$hdd_spindt" ] && hdd_spindt=0

hdd_apmoff=`nvram get hdd_apmoff`
[ -z "$hdd_apmoff" ] && hdd_apmoff=0


HDPARM_S="-S0"
HDPARM_B=""

case "$hdd_spindt" in
1)
	HDPARM_S="-S180"
	;;
2)
	HDPARM_S="-S241"
	;;
3)
	HDPARM_S="-S242"
	;;
4)
	HDPARM_S="-S243"
	;;
5)
	HDPARM_S="-S244"
	;;
6)
	HDPARM_S="-S245"
	;;
7)
	HDPARM_S="-S246"
	;;
8)
	HDPARM_S="-S247"
	;;
9)
	HDPARM_S="-S248"
	;;
esac

if [ $hdd_apmoff -ne 0 ] ; then
	HDPARM_B="-B254"
fi

if [ -z "$1" ] ; then
	for i in a b c d e f g h i k ; do
		removable=1
		if [ -e /sys/block/sd${i} ] ; then
			[ -r /sys/block/sd${i}/removable ] && removable=`cat /sys/block/sd${i}/removable`
			[ $removable -eq 0 ] && /sbin/hdparm $HDPARM_S $HDPARM_B /dev/sd${i}
		fi
	done
else
	if [ -e /sys/block/$1 ] ; then
		removable=1
		[ -r /sys/block/$1/removable ] && removable=`cat /sys/block/$1/removable`
		if [ $removable -eq 0 ] ; then
			if [ $hdd_spindt -gt 0 ] ; then
				logger -t hdparm "Set spindown timeout to device /dev/$1"
				/sbin/hdparm $HDPARM_S /dev/$1
			fi
			if [ $hdd_apmoff -ne 0 ] ; then
				logger -t hdparm "Set APM to device /dev/$1"
				/sbin/hdparm $HDPARM_B /dev/$1
			fi
		fi
	fi
fi

exit 0
