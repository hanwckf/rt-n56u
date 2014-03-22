#!/bin/sh

self_name="opt-umount.sh"

# check params
[ -z "$1" ] || [ -z "$2" ] && exit 1

# check /opt already unmounted
mountpoint -q /opt || exit 0

# check is this partition mounted to /opt
mountres=`grep ' /opt ' /proc/mounts | grep $1`
[ -z "$mountres" ] && exit 0

logger -t "${self_name}" "started [$@]"

# try to kill ipkg/opkg (may be running)
killall -q ipkg
killall -q opkg

# try to kill mc (may be running)
killall -q mc

# stop all services S* in /opt/etc/init.d
if [ -d /opt/etc/init.d ] ; then
	for i in `ls -r /opt/etc/init.d/S??* 2>/dev/null` ; do
		[ ! -x "${i}" ] && continue
		${i} stop
	done
fi

# check swap file exist
if [ -f /opt/.swap ] ; then
	swapoff /opt/.swap 2>/dev/null
	[ $? -eq 0 ] && logger -t "${self_name}" "Deactivate swap file /opt/.swap SUCCESS!"
fi

# restore home
rm -f /home/admin
ln -sf /home/root /home/admin

rm -f /etc/localtime

# flush buffers
sync

# unbinding
umount /opt
if [ $? -ne 0 ] ; then
	logger -t "${self_name}" "umount /opt FAILED! Device is busy?"
	sleep 1
	umount /opt 2>/dev/null
fi
