#!/bin/sh
# Test usage of loop devices

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

d1= f1=
cleanup_fn_()
{
  test -n "$d1" && losetup -d "$d1"
  rm -f "$f1"
}

f1=$(pwd)/1; d1=$(loop_setup_ "$f1") \
  || skip_ "is this partition mounted with 'nodev'?"

require_partitionable_loop_device_ $d1

# Expect this to succeed.
parted -s $d1 mklabel msdos > err 2>&1 || fail=1
compare /dev/null err || fail=1     # expect no output

# Create a partition
parted -s $d1 mkpart primary 1 10 > err 2>&1 || fail=1
compare /dev/null err || fail=1     # expect no output

Exit $fail
