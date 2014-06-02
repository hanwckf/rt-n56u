#!/bin/sh

self_name="opt-ipkg-upd.sh"

# check /opt mounted
if ! grep -q /opt /proc/mounts ; then
	echo "ERROR! Directory \"/opt\" not mounted!"
	exit 1
fi

export PATH=/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin

# check ipkg installed
if [ ! -f /opt/bin/ipkg ] ; then
	logger -t "${self_name}" "Installing optware ipkg...."
	mkdir -p /opt/tmp/ipkg
	ipkg.sh update
	if [ $? -eq 0 ] ; then
		ipkg.sh -force-defaults install ipkg-opt
		if [ $? -eq 0 ] ; then
			logger -t "${self_name}" "SUCCESS!"
		fi
	fi
	if [ $? -ne 0 ] ; then
		logger -t "${self_name}" "FAILED!"
	fi
fi

if [ -x /opt/bin/ipkg ] ; then
	logger -t "${self_name}" "Updating ipkg packages list..."
	ipkg update
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
	fi
fi
