#!/bin/sh
# ensure that parted can distinguish device map types: linear, multipath

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

require_root_
lvm_init_root_dir_

test "x$ENABLE_DEVICE_MAPPER" = xyes \
  || skip_ "no device-mapper support"

# Device maps names - should be random to not conflict with existing ones on
# the system
linear_=plinear-$$
mpath_=mpath-$$

d1= d2= d3=
f1= f2= f3=
cleanup_fn_() {
    dmsetup remove $linear_
    dmsetup remove $mpath_
    test -n "$d1" && losetup -d "$d1"
    test -n "$d2" && losetup -d "$d2"
    test -n "$d3" && losetup -d "$d3"
    rm -f "$f1" "$f2" "$f3";
}

f1=$(pwd)/1; d1=$(loop_setup_ "$f1") \
  || skip_ "is this partition mounted with 'nodev'?"

# setup: create loop devices
f2=$(pwd)/2 && d2=$(loop_setup_ "$f2") || fail=1
f3=$(pwd)/3 && d3=$(loop_setup_ "$f3") || fail=1

# This loop used to include "multipath", but lvm2 changed
# in such a way that that no longer works with loop devices.
# FIXME: use two scsi_debug devices instead.
for type in linear ; do

  case $type in
    linear)
      type_kwd=$linear_
      dmsetup_cmd="0 1024 linear $d1 0"
      ;;
    *)
      type_kwd=$mpath_
      dmsetup_cmd="0 1024 multipath 0 0 1 1 round-robin 0 2 0 $d2 $d3"
      ;;
  esac

  # setup: create a mapping
  echo "$dmsetup_cmd" | dmsetup create "$type_kwd" || fail=1
  dev="$DM_DEV_DIR/mapper/$type_kwd"

  # Create msdos partition table
  parted -s $dev mklabel msdos > out 2>&1 || fail=1
  compare /dev/null out || fail=1

  parted -s "$dev" print > out 2>&1 || fail=1
  sed 's/ $//' out > k && mv k out || fail=1 # Remove trailing blank.

  # Create expected output file.
  cat <<EOF >> exp || fail=1
Model: Linux device-mapper ($type) (dm)
Disk $dev: 524kB
Sector size (logical/physical): 512B/512B
Partition Table: msdos
Disk Flags:

Number  Start  End  Size  Type  File system  Flags

EOF

  compare exp out || fail=1
done

Exit $fail
