#!/bin/sh

existed=`cat /proc/partitions | grep $1 | wc -l`
if [ $existed == "0" ]; then
	exit 1
fi

# check filesystem type
FS_TYPE=""
blkid_info=`blkid /dev/$1`
if [ -n "`echo $blkid_info | grep 'TYPE=\"msdos\"'`" ] ; then
	FS_TYPE="msdos"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"vfat\"'`" ] ; then
	FS_TYPE="vfat"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"ntfs\"'`" ] ; then
	FS_TYPE="ntfs"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"ext4\"'`" ] ; then
	FS_TYPE="ext4"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"ext3\"'`" ] ; then
	FS_TYPE="ext3"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"ext2\"'`" ] ; then
	FS_TYPE="ext2"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"hfsplus\"'`" ] ; then
	FS_TYPE="hfsplus"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"hfs\"'`" ] ; then
	FS_TYPE="hfs"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"xfs\"'`" ] ; then
	FS_TYPE="xfs"
elif [ -n "`echo $blkid_info | grep 'TYPE=\"swap\"'`" ] ; then
	FS_TYPE="swap"
fi

if [ "$FS_TYPE" == "swap" ] ; then
	swap_used=`cat /proc/swaps | grep '^/dev/sd[a-z]' | grep 'partition' 2>/dev/null`
	if [ -z "$swap_used" ] ; then
		swapon /dev/$1
		if [ $? -eq 0 ] ; then
			nvram set swap_part_t=$1
			logger -t "automount" "Activate swap partition /dev/$1 SUCCESS!"
		fi
	fi
	
	# always return !=0 for swap
	exit 1
fi

mounted=`mount | grep /media/$2 | wc -l`

# mounted, try to umount
if [ $mounted -ge 1 ] ; then
	if ! umount "/media/$2"; then
		exit 1
	fi
	if ! rmdir "/media/$2"; then
		exit 1
	fi
fi

# lets mount under /media
if ! mkdir -p "/media/$2"; then
	exit 1
fi

kernel_26=`uname -r | grep \^2.6.`
achk_enable=`nvram get achk_enable`

if [ "$FS_TYPE" == "msdos" -o "$FS_TYPE" == "vfat" ] ; then
	modprobe -q vfat
	if [ "$achk_enable" != "0" ] && [ -x /sbin/dosfsck ] ; then
		/sbin/dosfsck -a -v "/dev/$1" > "/tmp/dosfsck_result_$1" 2>&1
	fi
	mount -t vfat "/dev/$1" "/media/$2" -o noatime,umask=0,iocharset=utf8,codepage=866,shortname=winnt
elif [ "$FS_TYPE" == "ntfs" ] ; then
	modprobe -q ufsd
	if [ "$achk_enable" != "0" ] && [ -x /sbin/chkntfs ] ; then
		/sbin/chkntfs -a -f --verbose "/dev/$1" > "/tmp/chkntfs_result_$1" 2>&1
	fi
	mount -t ufsd -o noatime,sparse,nls=utf8,force "/dev/$1" "/media/$2"
elif [ "$FS_TYPE" == "hfsplus" -o "$FS_TYPE" == "hfs" ] ; then
	if [ -n "$kernel_26" ] ; then
		modprobe -q ufsd
		mount -t ufsd -o noatime,nls=utf8,force "/dev/$1" "/media/$2"
	else
		modprobe -q $FS_TYPE
		mount -t $FS_TYPE -o noatime,umask=0,nls=utf8 "/dev/$1" "/media/$2"
	fi
elif [ "$FS_TYPE" == "ext4" -o "$FS_TYPE" == "ext3" -o "$FS_TYPE" == "ext2" ] ; then
	modprobe -q $FS_TYPE
	if [ "$achk_enable" != "0" ] && [ -x /sbin/e2fsck ] ; then
		/sbin/e2fsck -p -v "/dev/$1" > "/tmp/e2fsck_result_$1" 2>&1
	fi
	mount -t $FS_TYPE -o noatime "/dev/$1" "/media/$2"
elif [ "$FS_TYPE" == "xfs" ] ; then
	modprobe -q $FS_TYPE
	mount -t $FS_TYPE -o noatime "/dev/$1" "/media/$2"
fi

# check failed to mount, clean up mountpoint
mounted=`mount | grep /media/$2 | wc -l`
if [ $mounted == "0" ] ; then
	if ! rmdir "/media/$2"; then
		exit 1
	fi
fi

# call optware srcript
/usr/bin/opt-mount.sh "/dev/$1" "/media/$2" &


exit 0
