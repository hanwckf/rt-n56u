#!/bin/sh

if [ -f /opt/bin/ipkg ] ; then
	echo "WARNING! Optware detected. Please remove Optware first!"
	exit 1
fi

# install and update opkg manually
/usr/bin/opt-opkg-upd.sh
