#!/bin/sh
# exercise the align-check command

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
ss=$sector_size_

require_root_
require_scsi_debug_module_

grep '^#define USE_BLKID 1' "$CONFIG_HEADER" > /dev/null ||
  skip_ 'this system lacks a new-enough libblkid'

# create memory-backed device
scsi_debug_setup_ physblk_exp=3 lowest_aligned=7 sector_size=$ss > dev-name ||
  skip_ 'failed to create scsi_debug device'
scsi_dev=$(cat dev-name)
p1=${scsi_dev}1

parted -s $scsi_dev mklabel gpt || fail=1

i=60
while :; do
  parted -s $scsi_dev mkpart p1 ext2 ${i}s 800s || fail=1
  wait_for_dev_to_appear_ $p1 || fail=1
  parted -s $scsi_dev align-check min 1 > out 2>&1
  result=$?

  test $(expr $i % 8) = 7 && exp_result=0 || exp_result=1
  test $result = $exp_result || fail=1
  compare /dev/null out || fail=1

  parted -s $scsi_dev rm 1
  i=$(expr $i + 1)
  test $i = 70 && break

  # Wait up to 10s for the partition file to disappear.
  wait_for_dev_to_disappear_ $p1 10 || { fail=1; warn $p1 failed to disappear; }
done

Exit $fail
