#!/bin/sh

if [ ! -x /sbin/ubiattach ] || \
	[ ! -x /sbin/ubiformat ] || \
	[ ! -x /sbin/ubimkvol ] || \
	[ ! -x /sbin/ubidetach ]; then
  echo "Unable to find the UBI utils executables!" >&2
  exit 1
fi

rwfs="$(cat /proc/mtd |grep \"RWFS\")"

if [ -z "$rwfs" ]; then
	echo "RWFS partition not found!" >&2
	exit 1
fi

rwfs_idx="$(echo $rwfs |cut -d':' -f1|cut -c4-5)"

mountpoint="$(mount |grep /dev/ubi${rwfs_idx})"
if [ -n "$mountpoint" ]; then
	echo "/dev/ubi${rwfs_idx} alrealy mount!" >&2
	exit 1
fi

ubidetach -p /dev/mtd${rwfs_idx} > /dev/null 2>&1

mtd_write erase /dev/mtd${rwfs_idx}

ubiformat /dev/mtd${rwfs_idx} -y -q

if [ $? != 0 ]; then
	echo "Format RWFS partition (/dev/mtd${rwfs_idx}) to UBIFS FAILED!" >&2
	exit 1
fi

ubiattach -p /dev/mtd${rwfs_idx} -d ${rwfs_idx}

if [ $? != 0 ]; then
	echo "Attach UBIFS (/dev/mtd${rwfs_idx}) FAILED!" >&2
	exit 1
fi

ubimkvol /dev/ubi${rwfs_idx} -m -N rwfs

if [ $? = 0 ]; then
	echo -e "Create UBI volume done!\n\n"
	nvram set mtd_rwfs_mount=1 && nvram commit
	echo "Please reboot your router to automount RWFS partition at '/media/mtd_rwfs'."
	echo "If you want to automount RWFS to /opt, please create 'opt' directory in '/media/mtd_rwfs', then reboot the router."
	echo "If you want to setup Entware in /opt, please run 'nvram set optw_enable=2 && nvram commit', then run 'opt-start.sh' to download Entware installer."
else
	echo "Create UBI volume FAILED!" >&2
fi

ubidetach -p /dev/mtd${rwfs_idx}
