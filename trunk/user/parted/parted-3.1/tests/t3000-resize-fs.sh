#!/bin/sh
# exercise the resize library; FAT and HFS+ only

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

. "${srcdir=.}/init.sh"; path_prepend_ ../parted .
require_hfs_

require_root_
require_scsi_debug_module_
require_512_byte_sector_size_

ss=$sector_size_

start=63s
default_end=546147s
    new_end=530144s

# create memory-backed device
scsi_debug_setup_ dev_size_mb=550 > dev-name ||
  skip_test_ 'failed to create scsi_debug device'
dev=$(cat dev-name)

fail=0

parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# ensure that the disk is large enough
dev_n_sectors=$(parted -s $dev u s p|sed -n '2s/.* \([0-9]*\)s$/\1/p')
device_sectors_required=$(echo $default_end | sed 's/s$//')
# Ensure that $dev is large enough for this test
test $device_sectors_required -le $dev_n_sectors || fail=1

for fs_type in hfs+ fat32; do

  # create an empty $fs_type partition, cylinder aligned, size > 256 MB
  parted -a min -s $dev mkpart p1 $start $default_end > out 2>&1 || fail=1
  compare /dev/null out || fail=1

  # print partition table
  parted -m -s $dev u s p > out 2>&1 || fail=1

  # wait for new partition device to appear
  wait_for_dev_to_appear_ ${dev}1

  case $fs_type in
    fat32) mkfs_cmd='mkfs.vfat -F 32'; fsck='fsck.vfat -v';;
    hfs*) mkfs_cmd='mkfs.hfs';         fsck=fsck.hfs;;
    *) error "internal error: unhandled fs type: $fs_type";;
  esac

  # create the file system
  $mkfs_cmd ${dev}1 || fail=1

  # NOTE: shrinking is the only type of resizing that works.
  # resize that file system to be one cylinder (8MiB) smaller
  fs-resize ${dev}1 0 $new_end > out 2>&1 || fail=1
  # expect no output
  compare /dev/null out || fail=1

  # This is known to segfault with fsck.hfs from
  # Fedora 16's hfsplus-tools-332.14-12.fc15.x86_64.
  # You can build a working version from
  # git://cavan.codon.org.uk/hfsplus-tools.git

  # Skip the fsck.hfs test unless it understands the -v option.
  skip=0
  case $fs_type in
    hfs*) $fsck -v || { warn_ skipping $fsck test; skip=1; } ;; esac

  if test $skip = 0; then
    $fsck ${dev}1 > out || fail=1
    cat out
    # Oops.  Currently, fsck.hfs reports this:
    # Executing fsck_hfs (version 540.1-Linux).
    # ** Checking non-journaled HFS Plus Volume.
    #    The volume name is untitled
    # ** Checking extents overflow file.
    # ** Checking catalog file.
    # ** Checking multi-linked files.
    # ** Checking catalog hierarchy.
    # ** Checking volume bitmap.
    #    Volume bitmap needs minor repair for orphaned blocks
    # ** Checking volume information.
    #    Invalid volume free block count
    #    (It should be 67189 instead of 65197)
    #    Volume header needs minor repair
    # (2, 0)
    # FIXME: This means the HFS resizing code is wrong.

    # FIXME: parse "out" for FS size and verify that it's the new, smaller size
  fi

  # Remove the partition explicitly, so that mklabel doesn't evoke a warning.
  parted -s $dev rm 1 || fail=1

  # Create a clean partition table for the next iteration.
  parted -s $dev mklabel gpt > out 2>&1 || fail=1
  # expect no output
  compare /dev/null out || fail=1

done

Exit $fail
