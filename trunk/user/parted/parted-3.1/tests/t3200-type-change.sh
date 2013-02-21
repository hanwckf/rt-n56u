#!/bin/sh
# Ensure parted doesn't change the type of a partition to match its FS.

# Copyright (C) 2009-2012 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. "${srcdir=.}/init.sh"; path_prepend_ ../parted

require_root_
require_scsi_debug_module_

grep '^#define USE_BLKID 1' "$CONFIG_HEADER" > /dev/null ||
  skip_ 'this system lacks a new-enough libblkid'

# create memory-backed device
scsi_debug_setup_ dev_size_mb=550 > dev-name ||
  skip_ 'failed to create scsi_debug device'
scsi_dev=$(cat dev-name)

# Create an empty partition of "type fat32", and then create an actual
# ext2 partition in it.  Then "unset" the already unset LVM flag on that
# partition and ensure that parted doesn't "helpfully" change the partition
# type to match the newly-detected FS type.

parted -s $scsi_dev mklabel msdos mkpart primary fat32 64s 80000s || fail=1

parted -s $scsi_dev u s p

p1=${scsi_dev}1
wait_for_dev_to_appear_ $p1 || fail=1
mkfs.ext2 $p1 || fail=1

# print as hex, the type of the first partition
msdos_p1_type() { od -An --skip=450 -N1 -tx1 "$1"; }

# Initially, it is 0x0c (FAT32).
type=$(msdos_p1_type $scsi_dev) || fail=1
type=${type# } # remove leading space
case $type in
  0c) ;;
  *) fail_ "expected initial type of 0c (FAT32)";;
esac

parted -s $scsi_dev u s p
parted -s $scsi_dev set 1 lvm off || fail=1

# Before parted-2.1, the above would mistakenly change it to 0x83,
# to match the file system now residing in that partition.
type=$(msdos_p1_type $scsi_dev) || fail=1
type=${type# } # remove leading space
case $type in
  0c) ;;
  *) fail_ "parted changed the type of partition 1 from 0c to $type";;
esac

# Ensure that setting the "lvm" flag still works.
parted -s $scsi_dev set 1 lvm on || fail=1
parted -s $scsi_dev u s p > out || fail=1
grep lvm out || { fail=1; cat out; }

Exit $fail
