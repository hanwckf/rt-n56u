#!/bin/sh
# ensure that parted names partitions on dm disks correctly

# Copyright (C) 2011-2012 Free Software Foundation, Inc.

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
lvm_init_root_dir_

test "x$ENABLE_DEVICE_MAPPER" = xyes \
  || skip_ "no device-mapper support"

# Device maps names - should be random to not conflict with existing ones on
# the system
linear_=plinear-$$
linear2_=plinear-$$foo

d1= d2=
f1= f2=
cleanup_fn_() {
    dmsetup remove ${linear_}p1
    dmsetup remove $linear_
    dmsetup remove ${linear2_}1
    dmsetup remove $linear2_
    test -n "$d1" && losetup -d "$d1"
    test -n "$d2" && losetup -d "$d2"
    rm -f "$f1 $f2";
}

# create a file of size N bytes
N=10M

# create the test file
f1=$(pwd)/1; dd if=/dev/null of=$f1 bs=1 seek=$N 2> /dev/null || fail=1
f2=$(pwd)/2; dd if=/dev/null of=$f2 bs=1 seek=$N 2> /dev/null || fail=1

d1=$(loop_setup_ "$f1") \
  || skip_ "is this partition mounted with 'nodev'?"

d2=$(loop_setup_ "$f2") \
  || skip_ "is this partition mounted with 'nodev'?"

dmsetup_cmd="0 `blockdev --getsz $d1` linear $d1 0"
# setup: create a mapping
echo "$dmsetup_cmd" | dmsetup create "$linear_" || fail=1
dev="$DM_DEV_DIR/mapper/$linear_"

# Create msdos partition table
parted -s $dev mklabel msdos > out 2>&1 || fail=1
compare /dev/null out || fail=1

parted -s $dev mkpart primary fat32 1m 5m > out 2>&1 || fail=1
compare /dev/null out || fail=1

#make sure device name is correct
test -e ${dev}p1 || fail=1

#repeat on name not ending in a digit
# setup: create a mapping
echo "$dmsetup_cmd" | dmsetup create "$linear2_" || fail=1
dev="$DM_DEV_DIR/mapper/$linear2_"

# Create msdos partition table
parted -s $dev mklabel msdos > out 2>&1 || fail=1
compare /dev/null out || fail=1

parted -s $dev mkpart primary fat32 1m 5m > out 2>&1 || fail=1
compare /dev/null out || fail=1

#make sure device name is correct
test -e ${dev}1 || fail=1

Exit $fail
