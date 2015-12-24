#!/bin/sh

self_name="Entware"

# check /opt mounted
if ! grep -q /opt /proc/mounts ; then
	echo "ERROR! Directory \"/opt\" not mounted!"
	exit 1
fi

export PATH=/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin


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

	logger -t "${self_name}" "Creating folders..."
	for folder in bin etc/init.d lib/opkg sbin share tmp usr var/log var/lock var/run ; do
		if [ -d "/opt/$folder" ] ; then
			logger -t "${self_name}" "Warning: Folder /opt/$folder exists! If something goes wrong please clean /opt folder and try again."
		else
			mkdir -p /opt/$folder
		fi
	done

	URL=http://pkg.entware.net/binaries/mipsel/installer
	dl $URL/opkg /opt/bin/opkg x
	dl $URL/opkg.conf /opt/etc/opkg.conf
	dl $URL/profile /opt/etc/profile x
	dl $URL/rc.func /opt/etc/init.d/rc.func
	dl $URL/rc.unslung /opt/etc/init.d/rc.unslung x

	logger -t "${self_name}" "Updating opkg packages list..."
	opkg update
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	logger -t "${self_name}" "Installing ldconfig findutils..."
	opkg install ldconfig findutils
	if [ $? -eq 0 ] ; then
		logger -t "${self_name}" "SUCCESS!"
	else
		logger -t "${self_name}" "FAILED!"
		exit 1
	fi
	ln -sf /etc/TZ /opt/etc/TZ
	ldconfig > /dev/null 2>&1
	logger -t "${self_name}" "Congratulations!"
	logger -t "${self_name}" "If there are no errors above then Entware successfully initialized."
	logger -t "${self_name}" "Found a Bug? Please report at https://github.com/Entware-ng/Entware-ng/issues"
fi
