#!/bin/bash

THISDIR=`pwd`

. ${THISDIR}/../.config

if [ "$CONFIG_LINUXDIR" = "linux-3.0.x" ] ; then
	CONFIG_CROSS_COMPILER_PATH="$CONFIG_TOOLCHAIN_DIR/toolchain-3.0.x/bin"
elif [ "$CONFIG_LINUXDIR" = "linux-3.4.x" ] ; then
	CONFIG_CROSS_COMPILER_PATH="$CONFIG_TOOLCHAIN_DIR/toolchain-3.4.x/bin"
fi

export PATH="${CONFIG_CROSS_COMPILER_PATH}:$PATH"

build_dir="${THISDIR}/headers"

rm -rf ${build_dir}
mkdir -p ${build_dir}/usr

cd ../${CONFIG_LINUXDIR}
make O=${build_dir} ARCH=mips HOSTCC=cc CROSS_COMPILE=mipsel-linux-uclibc- INSTALL_HDR_PATH=${build_dir}/usr headers_install

find ${build_dir}/usr -type f -name ".*" -exec rm "{}" \;
