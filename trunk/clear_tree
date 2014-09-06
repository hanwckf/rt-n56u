#!/bin/sh

ROOTDIR=`pwd`
export ROOTDIR=$ROOTDIR

if [ ! -f "$ROOTDIR/.config" ] ; then
	cp -fv "$ROOTDIR/configs/templates/n56u_dlna.config" "$ROOTDIR/.config"
fi

if [ ! -f "$ROOTDIR/.config" ] ; then
	echo "Project config file .config not found! Terminate."
	exit 1
fi

echo "-------------CLEAN-ALL---------------"
rm -rf $ROOTDIR/stage
make clean

rm -rfv $ROOTDIR/romfs
rm -rfv $ROOTDIR/images
rm -rfv $ROOTDIR/stage
