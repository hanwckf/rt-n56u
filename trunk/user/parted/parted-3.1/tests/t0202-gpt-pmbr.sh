#!/bin/sh
# Preserve first 446B of the Protected MBR for gpt partitions.

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

dev=loop-file
bootcode_size=446

dd if=/dev/null of=$dev bs=1 seek=1M || framework_failure

# create a GPT partition table
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# Fill the first $bootcode_size bytes with 0's.
# This affects only the protective MBR, so doesn't affect validity of gpt table.
printf %0${bootcode_size}d 0 > in || fail=1
dd of=$dev bs=1 seek=0 count=$bootcode_size conv=notrunc < in || fail=1

parted -s $dev p || fail=1

# create a GPT partition table on top of the existing one.
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# Extract the first $bootcode_size Bytes after GPT creation
dd if=$dev of=after bs=1c count=$bootcode_size > /dev/null 2>&1 || fail=1

# Compare the before and after
compare in after || fail=1

Exit $fail
