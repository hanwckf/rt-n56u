#!/bin/sh

existed=`cat /proc/partitions | grep $1 | wc -l`
if [ $existed == "0" ]; then
	exit 1
fi

dev_full="/dev/$1"

# check filesystem type
ID_FS_TYPE=""
ID_FS_UUID=""
ID_FS_LABEL=""
eval `/sbin/blkid -o udev $dev_full`

if [ "$ID_FS_TYPE" == "swap" ] ; then
	swap_used=`cat /proc/swaps | grep '^/dev/sd[a-z]' | grep 'partition' 2>/dev/null`
	if [ -z "$swap_used" ] ; then
		swapon $dev_full
		if [ $? -eq 0 ] ; then
			nvram set swap_part_t=$1
			logger -t "automount" "Activate swap partition $dev_full SUCCESS!"
		fi
	fi
	
	# always return !=0 for swap
	exit 1
fi

dev_label=`echo "$ID_FS_LABEL" | tr -d '\/:*?"<>|~$^' | sed 's/ /_/g'`
[ "$dev_label" == "NO_NAME" ] && dev_label=""

mnt_legacy="/media/$2"
if [ -z "$dev_label" ] ; then
	dev_mount="$mnt_legacy"
else
	dev_mount="/media/$dev_label"
fi

# if mounted, try to umount
if mountpoint -q "$dev_mount" ; then
	if ! umount -f "$dev_mount" ; then
		dev_mount="$mnt_legacy"
		if mountpoint -q "$dev_mount" ; then
			if ! umount -f "$dev_mount" ; then
				logger -t "automount" "Unable to prepare mountpoint $dev_mount!"
				exit 1
			fi
		fi
	fi
fi

# lets mount under /media
if ! mkdir -p "$dev_mount" ; then
	logger -t "automount" "Unable to create mountpoint $dev_mount!"
	exit 1
fi

kernel_26=`uname -r | grep \^2.6.`
achk_enable=`nvram get achk_enable`

if [ "$ID_FS_TYPE" == "msdos" -o "$ID_FS_TYPE" == "vfat" ] ; then
	modprobe -q vfat
	if [ "$achk_enable" != "0" ] && [ -x /sbin/dosfsck ] ; then
		/sbin/dosfsck -a -v "$dev_full" > "/tmp/dosfsck_result_$1" 2>&1
	fi
	mount -t vfat "$dev_full" "$dev_mount" -o noatime,umask=0,iocharset=utf8,codepage=866,shortname=winnt
elif [ "$ID_FS_TYPE" == "ntfs" ] ; then
	modprobe -q ufsd
	if [ "$achk_enable" != "0" ] && [ -x /sbin/chkntfs ] ; then
		/sbin/chkntfs -a -f --verbose "$dev_full" > "/tmp/chkntfs_result_$1" 2>&1
	fi
	mount -t ufsd -o noatime,sparse,nls=utf8,force "$dev_full" "$dev_mount"
elif [ "$ID_FS_TYPE" == "hfsplus" -o "$ID_FS_TYPE" == "hfs" ] ; then
	if [ -n "$kernel_26" ] ; then
		modprobe -q ufsd
		mount -t ufsd -o noatime,nls=utf8,force "$dev_full" "$dev_mount"
	else
		modprobe -q $ID_FS_TYPE
		mount -t $ID_FS_TYPE -o noatime,umask=0,nls=utf8 "$dev_full" "$dev_mount"
	fi
elif [ "$ID_FS_TYPE" == "ext4" -o "$ID_FS_TYPE" == "ext3" -o "$ID_FS_TYPE" == "ext2" ] ; then
	modprobe -q $ID_FS_TYPE
	if [ "$achk_enable" != "0" ] && [ -x /sbin/e2fsck ] ; then
		/sbin/e2fsck -p -v "$dev_full" > "/tmp/e2fsck_result_$1" 2>&1
	fi
	mount -t $ID_FS_TYPE -o noatime "$dev_full" "$dev_mount"
elif [ "$ID_FS_TYPE" == "xfs" ] ; then
	modprobe -q $ID_FS_TYPE
	mount -t $ID_FS_TYPE -o noatime "$dev_full" "$dev_mount"
fi

# check failed to mount, clean up mountpoint
if ! mountpoint -q "$dev_mount" ; then
	if [ -n "$ID_FS_TYPE" ] ; then
		logger -t "automount" "Unable to mount device $dev_full ($ID_FS_TYPE) to $dev_mount!"
	fi
	rmdir "$dev_mount"
	exit 1
fi

chmod 777 "$dev_mount"

# check ACL files
/sbin/test_share "$dev_mount"

# call optware srcript
/usr/bin/opt-mount.sh "$dev_full" "$dev_mount" &

exit 0
