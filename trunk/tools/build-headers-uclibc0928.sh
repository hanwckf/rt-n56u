#!/bin/bash

THISDIR=`pwd`

. ${THISDIR}/../.config

build_dir="${THISDIR}/headers"

rm -rf ${build_dir}
mkdir -p ${build_dir}/include

cd ../${CONFIG_LINUXDIR}

KERNEL_HEADERS_SUBDIRS="asm asm-generic linux"
for i in $KERNEL_HEADERS_SUBDIRS ; do
	echo "$i"
	cp -pLR "include/$i"  ${build_dir}/include
done
