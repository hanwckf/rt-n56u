#!/bin/sh

mtd_part_name="Storage"
mtd_part_dev="/dev/mtdblock5"
mtd_part_size=65536
dir_storage="/etc/storage"
tmp="/tmp/.storage_tar"
hsh="/tmp/hashes/storage_md5"

func_get_mtd()
{
	local mtd_char mtd_index
	mtd_char=`cat /proc/mtd | grep \"$mtd_part_name\" | awk {' print $1 '} | cut -d':' -f1`
	if [ -n "$mtd_char" ] ; then
		mtd_index=${mtd_char/mtd/}
		if [ -n "$mtd_index" ] && [ $mtd_index -ge 4 ] ; then
			mtd_part_dev="/dev/mtdblock${mtd_index}"
		fi
	fi
}

func_load()
{
	local fsz
	
	mkdir -p -m 755 $dir_storage
	echo "Loading files from mtd partition \"$mtd_part_dev\""
	bzcat $mtd_part_dev > $tmp 2>/dev/null
	fsz=`stat -c %s $tmp 2>/dev/null`
	if [ -n "$fsz" ] && [ $fsz -gt 0 ] ; then
		md5sum $tmp > $hsh
		tar xf $tmp -C $dir_storage 2>/dev/null
		echo "Done."
	else
		rm -f $hsh
		echo "Error! Invalid storage data"
	fi
	rm -f $tmp
}

func_save()
{
	local fsz tbz2
	
	mkdir -p -m 755 $dir_storage
	echo "Save files to mtd partition \"$mtd_part_dev\""
	
	tbz2="${tmp}.bz2"
	rm -f $tmp
	rm -f $tbz2
	cd $dir_storage
	tar cf $tmp * 2>/dev/null
	cd - >>/dev/null
	md5sum -c -s $hsh 2>/dev/null
	if [ $? -eq 0 ] ; then
		echo "Storage hash is not changed, skip write to mtd partition. Exit."
		rm -f $tmp
		return 0
	fi
	[ -f "$tmp" ] && md5sum $tmp > $hsh
	bzip2 -9 $tmp 2>/dev/null
	fsz=`stat -c %s $tbz2 2>/dev/null`
	if [ -n "$fsz" ] && [ $fsz -gt 0 ] && [ $fsz -lt $mtd_part_size ] ; then
		mtd_write write $tbz2 $mtd_part_name
		if [ $? -eq 0 ] ; then
			sync
			echo "Done."
		else
			echo "Error! mtd write FAILED"
		fi
	else
		echo "Error! Invalid storage data size: $fsz"
	fi
	rm -f $tmp
	rm -f $tbz2
}

func_erase()
{
	echo "Erase mtd partition \"$mtd_part_dev\""
	
	mtd_write erase $mtd_part_name
	if [ $? -eq 0 ] ; then
		rm -f $hsh
		rm -rf $dir_storage
		mkdir -p -m 755 $dir_storage
		echo "Done."
	else
		echo "Error! mtd erase FAILED"
	fi
}

case "$1" in
load)
	func_get_mtd
	func_load
	;;
save)
	func_get_mtd
	func_save
	;;
erase)
	func_get_mtd
	func_erase
	;;
*)
	echo "Usage: $0 {load|save|erase}"
	exit 1
	;;
esac
