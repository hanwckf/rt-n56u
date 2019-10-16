#!/bin/sh
[ -z "$UNSQSHFS_TOOL" ] && echo "UNSQSHFS_TOOL is not set" >&2 && exit 1
[ -z "$ROOTDIR" ] && echo "ROOTDIR is not set" >&2 && exit 1

. $ROOTDIR/.config

UNPACK_TRX_DIR="$ROOTDIR/merge/${CONFIG_FIRMWARE_PRODUCT_ID}"
[ -d "$UNPACK_TRX_DIR" ] && exit 0

mkdir -p "$UNPACK_TRX_DIR"

TARGET="$ROOTDIR/merge/${CONFIG_FIRMWARE_PRODUCT_ID}.trx"
ROOTFS="$UNPACK_TRX_DIR/rootfs"

offset1=`grep -oba hsqs $TARGET | grep -oP '[0-9]*(?=:hsqs)'`
offset2=`wc -c $TARGET | grep -oP '[0-9]*(?= )'`
size2=`expr $offset2 - $offset1`
#dd if="$TARGET" of="$UNPACK_TRX_DIR/uboot_header.bin" bs=1 ibs=1 count=64
dd if="$TARGET" of="$UNPACK_TRX_DIR/kernel.bin" bs=1 ibs=1 skip=64 count=$((offset1-64))
dd if="$TARGET" of="$UNPACK_TRX_DIR/secondchunk.bin" bs=1 ibs=1 count="$size2" skip="$offset1"

$UNSQSHFS_TOOL -d "$ROOTFS" "$UNPACK_TRX_DIR/secondchunk.bin"
rm -f "$UNPACK_TRX_DIR/secondchunk.bin"
