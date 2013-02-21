#!/bin/sh
# Ensure that a whole-disk FAT partition is detected.

# Copyright (C) 2010-2012 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

. "${srcdir=.}/init.sh"; path_prepend_ ../parted
require_512_byte_sector_size_

dev_file=dev-file

echo '1:0s:81919s:81920s:fat16::;' > exp || framework_failure_
dd if=/dev/null of=$dev_file bs=1 seek=40M || framework_failure_
mkfs.vfat -F 16 $dev_file || skip_ "mkfs.vfat failed"

parted -m -s $dev_file u s print > out 2>&1 || fail=1
grep '^1:' out > k; mv k out
compare exp out || fail=1

Exit $fail
