#!/bin/sh
# Probe Ext2, Ext3 and Ext4 file systems

# Copyright (C) 2008-2012 Free Software Foundation, Inc.

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
require_512_byte_sector_size_

dev=loop-file
ss=$sector_size_

for type in ext2 ext3 ext4 nilfs2; do

  ( mkfs.$type -V ) >/dev/null 2>&1 \
      || { warn_ "$ME: no $type support"; continue; }

  case $type in ext*) n_sectors=8000 force=-F;;
      *) n_sectors=$((257*1024)) force=;; esac

  # create an $type file system
  dd if=/dev/zero of=$dev bs=$ss count=$n_sectors >/dev/null || fail=1
  mkfs.$type $force $dev || { warn_ $ME: mkfs.$type failed; fail=1; continue; }

  # probe the $type file system
  parted -m -s $dev u s print >out 2>&1 || fail=1
  grep '^1:.*:'$type'::;$' out || { cat out; fail=1; }

done

# Some features should indicate ext4 by themselves.
for feature in uninit_bg flex_bg; do
  # create an ext3 file system
  dd if=/dev/zero of=$dev bs=1024 count=4096 >/dev/null || fail=1
  mkfs.ext3 -F $dev >/dev/null || skip_ "mkfs.ext3 failed"

  # set the feature
  tune2fs -O $feature $dev || skip_ "tune2fs failed"

  # probe the file system, which should now be ext4
  parted -m -s $dev u s print >out 2>&1 || fail=1
  grep '^1:.*:ext4::;$' out || fail=1
done

Exit $fail
