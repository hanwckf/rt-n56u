#!/bin/sh

self_name="Entware"

# check /opt mounted
if ! grep -q /opt /proc/mounts ; then
	echo "ERROR! Directory \"/opt\" not mounted!"
	exit 1
fi

export PATH=/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin
INSTALLER=/tmp/entware_installer.sh

dl () {
	# $1 - URL to download
	# $2 - place to store
	# $3 - 'x' if should be executable
	logger -t "${self_name}" "Downloading $2..."
	wget -q $1 -O $2
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	[ -z "$3" ] || chmod +x $2
}

# check opkg installed
if [ ! -f /opt/bin/opkg ] ; then
	logger -t "${self_name}" "Installing entware opkg...."
	wget -q http://bin.entware.net/mipselsf-k3.4/installer/generic.sh -O $INSTALLER
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	chmod +x $INSTALLER
	sh $INSTALLER >> $INSTALLER.log 2>&1
	ln -sf /etc/TZ /opt/etc/TZ
	logger -t "${self_name}" "Updating opkg packages list..."
	opkg update
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED! See $INSTALLER.log for details."
		exit 1
	fi
	rm -f $INSTALLER $INSTALLER.log
	logger -t "${self_name}" "Congratulations!"
	logger -t "${self_name}" "If there are no errors above then Entware successfully initialized."
	logger -t "${self_name}" "Found a Bug? Please report at https://github.com/Entware/Entware/issues"
fi
