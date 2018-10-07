#!/bin/sh

# check env
if [ -z "$ROMFSDIR" ]; then
	echo "ROMFSDIR is not set" >&2
	exit 1
fi

if [ -z "$ROOTDIR" ]; then
	echo "ROOTDIR is not set" >&2
	exit 1
fi

if [ ! -x "$STRIPTOOL" ] ; then
	echo "STRIPTOOL is not set" >&2
	exit 1
fi

if [ ! -x "$OBJCOPY" ] ; then
	echo "OBJCOPY is not set" >&2
	exit 1
fi

NON_STRIPS_LIB=`find ${ROMFSDIR}/lib ${ROMFSDIR}/usr/lib -type f -name "*.so*"; `
KERNEL_MODULES=`find ${ROMFSDIR}/lib/modules -type f -name "*.ko"`;

# add busybox (auto-installed w/o romfs-inst.sh)
NON_STRIPS_LIB="${NON_STRIPS_LIB} ${ROMFSDIR}/bin/busybox"

echo -----------------------------------STRIP LIB----------------------------------
for i in $NON_STRIPS_LIB; do
	echo $i;
	${OBJCOPY} --strip-debug --strip-unneeded $i $i
done
if [ -n "$NON_STRIPS_LIB" ]; then
	${STRIPTOOL} $NON_STRIPS_LIB
	${STRIPTOOL} -R .comment -R .note --strip-debug --strip-unneeded $NON_STRIPS_LIB
	if [ -x "${SSTRIP_TOOL}" ] ; then
		${SSTRIP_TOOL} $NON_STRIPS_LIB
	fi
fi
if [ "$CONFIG_WITHOUT_KERNEL" != "y" ]; then
echo -----------------------------------STRIP MOD----------------------------------
for i in $KERNEL_MODULES; do
	echo $i;
	${OBJCOPY} --strip-debug --strip-unneeded $i $i
done
if [ -n "$KERNEL_MODULES" ]; then
	${STRIPTOOL} -R .comment -R .note --strip-debug --strip-unneeded $KERNEL_MODULES
fi
fi
sync
echo ------------------------------LIB STRIP AND COPY OK---------------------------
