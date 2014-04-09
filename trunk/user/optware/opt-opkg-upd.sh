#!/bin/sh

self_name="Entware"

# check /opt mounted
if ! grep -q /opt /proc/mounts ; then
	echo "ERROR! Directory \"/opt\" not mounted!"
	exit 1
fi

export PATH=/opt/sbin:/usr/sbin:/sbin:/opt/bin:/usr/bin:/bin

# check opkg installed
if [ ! -f /opt/bin/opkg ] ; then
	logger -t "${self_name}" "Installing entware opkg...."

	logger -t "${self_name}" "Checking for prerequisites and creating folders..."
	for folder in include share tmp usr ; do
		if [ -d "/opt/$folder" ] ; then
			logger -t "${self_name}" "Warning: Folder /opt/$folder exists!"
			logger -t "${self_name}" "Warning: If something goes wrong please clean /opt folder and try again."
		else
			mkdir /opt/$folder
		fi
	done
	[ -d "/opt/lib/opkg" ] || mkdir -p /opt/lib/opkg
	[ -d "/opt/var/lock" ] || mkdir -p /opt/var/lock
	[ -d "/opt/var/run" ] || mkdir -p /opt/var/run

	logger -t "${self_name}" "Opkg package manager deployment..."
	cd /opt/bin
	logger -t "${self_name}" "Downloading opkg..."
	wget http://entware.wl500g.info/binaries/entware/installer/opkg
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	chmod +x /opt/bin/opkg
	cd /opt/etc
	logger -t "${self_name}" "Downloading opkg.conf..."
	wget http://entware.wl500g.info/binaries/entware/installer/opkg.conf
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	logger -t "${self_name}" "Basic packages installation..."
	logger -t "${self_name}" "Updating opkg packages list..."
	/opt/bin/opkg update
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	logger -t "${self_name}" "Installing uclibc-opt..."
	/opt/bin/opkg install uclibc-opt
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	logger -t "${self_name}" "Congratulations!"
	logger -t "${self_name}" "If there are no errors above then Entware successfully initialized."
	logger -t "${self_name}" "Found a Bug? Please report at https://github.com/Entware/entware/issues"
fi
