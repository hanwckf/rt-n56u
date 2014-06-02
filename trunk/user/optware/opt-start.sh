#!/bin/sh

self_name="opt-start.sh"

optw_enable=`nvram get optw_enable`
[ "$optw_enable" != "1" -a "$optw_enable" != "2" ] && exit 0

# check /opt is mounted
mountpoint -q /opt || exit 0

logger -t "${self_name}" "call /opt/etc/init.d"

# extend path to /opt
export PATH=/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin

# start all services S* in /opt/etc/init.d
for i in `ls /opt/etc/init.d/S??* 2>/dev/null` ; do
	[ ! -x "${i}" ] && continue
	${i} start
done

# install and update ipkg/opkg in background
if [ "$optw_enable" == "1" ] ; then
	if [ ! -f /opt/bin/ipkg ] ; then
		if [ -f /opt/bin/opkg ] ; then
			logger -t "Optware:" "WARNING! Entware detected (/opt). Please remove Entware first!"
			exit 0
		fi
		/usr/bin/opt-ipkg-upd.sh
	fi
elif [ "$optw_enable" == "2" ] ; then
	if [ ! -f /opt/bin/opkg ] ; then
		if [ -f /opt/bin/ipkg ] ; then
			logger -t "Entware:" "WARNING! Optware detected (/opt). Please remove Optware first!"
			exit 0
		fi
		/usr/bin/opt-opkg-upd.sh
	fi
fi

