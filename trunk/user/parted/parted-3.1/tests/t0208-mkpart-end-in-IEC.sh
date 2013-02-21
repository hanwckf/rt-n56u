#!/bin/sh
# Make sure parted mkpart ends the partition one sector before the specified
# value if end is specified with IEC units.

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

require_512_byte_sector_size_
n_mbs=8
dev=dev-file

dd if=/dev/null of=$dev bs=1M seek=$n_mbs || fail=1
# create 1st partition
parted --align=none -s $dev mklabel gpt mkpart p1 1MiB 2MiB > err 2>&1 || fail=1
compare /dev/null err || fail=1  # expect no output
#parted -m -s $dev u s p > exp || fail=1

# create 2nd partition - they should not overlap
# this time specify default unit
parted --align=none -s $dev unit MiB mkpart p2 2 3 > err 2>&1 || fail=1
compare /dev/null err || fail=1  # expect no output

# create 3rd partition - expect no overlap
# specify default unit, but explicitly override it
parted --align=none -s $dev unit TB mkpart p3 3MiB 4MiB > err 2>&1 || fail=1
compare /dev/null err || fail=1  # expect no output

# Specify default unit of MiB, yet use explicit ending sector number.
parted --align=none -s $dev unit MiB mkpart p4 4MiB 10239s > err 2>&1 || fail=1
compare /dev/null err || fail=1  # expect no output

# check boundaries of the partitions
parted -m -s $dev u s p > out || fail=1

# prepare expected output
cat <<EOF > exp || framework_failure
1:2048s:4095s:2048s::p1:;
2:4096s:6143s:2048s::p2:;
3:6144s:8191s:2048s::p3:;
4:8192s:10239s:2048s::p4:;
EOF

# compare expected and actual outputs
sed -e "1,2d" out > k; mv k out
compare exp out || fail=1

Exit $fail
