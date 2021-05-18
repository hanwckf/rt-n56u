#!/bin/sh

depmod_bin="/sbin/depmod"

# check env
if [ -z "${ROOTDIR}" ]; then
	echo "ROOTDIR is not set!"
	exit 1
fi

if [ -z "${KERNELRELEASE}" ]; then
	echo "KERNELRELEASE is not set!"
	exit 1
fi

if [ -z "${INSTALL_MOD_PATH}" ]; then
	echo "INSTALL_MOD_PATH is not set!"
	exit 1
fi

if [ ! -x "$depmod_bin" ]; then
	echo "$depmod_bin is not found! Please install package module-init-tools (kmod)"
	exit 1
fi

if [ "$CONFIG_FIRMWARE_INCLUDE_IPSET" = "y" ] ; then
	ipset_dir="${ROOTDIR}/user/ipset/ipset-6.x/kernel/net/netfilter"
	mkdir -p "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/net/netfilter/ipset"
	cp -f "$ipset_dir/xt_set.ko" "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/net/netfilter"
	cp -f "$ipset_dir/ipset/"*.ko "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/net/netfilter/ipset"
fi

if [ "$CONFIG_FIRMWARE_INCLUDE_NAPT66" = "y" ] ; then
	napt66_dir="${ROOTDIR}/user/napt66/napt66-v1.1"
	mkdir -p "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/napt66"
	cp -f "$napt66_dir/napt66.ko" "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/napt66"
fi

if [ "$CONFIG_FIRMWARE_INCLUDE_SFE" = "y" ] ; then
	sfe_dir="${ROOTDIR}/user/shortcut-fe/shortcut-fe"
	mkdir -p "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/sfe"
	cp -f "$sfe_dir/"*.ko "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/sfe"
fi

if [ "$CONFIG_FIRMWARE_INCLUDE_ANTFS" = "y" ] ; then
	antfs_dir="${ROOTDIR}/user/antfs"
	mkdir -p "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/antfs"
	cp -f "$antfs_dir/"antfs.ko "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/antfs"
fi

# call depmod
$depmod_bin -ae -F System.map -b "${INSTALL_MOD_PATH}" -r ${KERNELRELEASE}

# clear unneeded depmod files
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.alias"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.alias.bin"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.dep.bin"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.devname"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.softdep"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.symbols"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.symbols.bin"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.builtin"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.builtin.bin"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.builtin.alias.bin"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.order"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/build"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/source"
