#!/bin/sh
# check physical sector size as reported by 'print'

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

echo 'Sector size (logical/physical): 4096B/4096B' > exp || framework_failure

# create memory-backed device
scsi_debug_setup_ dev_size_mb=8 sector_size=4096 > dev-name ||
  skip_ 'failed to create scsi_debug device'
scsi_dev=$(cat dev-name)

# create partition table and print
parted -s $scsi_dev mklabel gpt print > out 2>&1 || fail=1
grep '^Sector' out > k 2>&1 || fail=1
mv k out || fail=1

compare exp out || fail=1

Exit $fail
