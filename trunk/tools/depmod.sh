#!/bin/sh

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

# copy ufsd.ko
if [ "$CONFIG_FIRMWARE_ENABLE_UFSD" = "y" ] ; then
	mkdir -p "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/fs/ufsd"
	cp -f "${ROOTDIR}/proprietary/ufsd/ufsd-${KERNELRELEASE}.ko" "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/fs/ufsd/ufsd.ko"
fi

if [ "$CONFIG_FIRMWARE_INCLUDE_IPSET" = "y" ] ; then
	ipset_dir="${ROOTDIR}/user/ipset/ipset-6.x/kernel/net/netfilter"
	mkdir -p "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/net/netfilter/ipset"
	cp -f "$ipset_dir/xt_set.ko" "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/net/netfilter"
	cp -f "$ipset_dir/ipset/"*.ko "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/kernel/net/netfilter/ipset"
fi

# call depmod
sudo /sbin/depmod -ae -F System.map -b "${INSTALL_MOD_PATH}" -r ${KERNELRELEASE}

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
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/modules.order"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/build"
rm -f "${INSTALL_MOD_PATH}/lib/modules/${KERNELRELEASE}/source"
