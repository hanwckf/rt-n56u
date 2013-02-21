#!/bin/sh
# Ensure parted preserves bootcode in extended partition.

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

require_512_byte_sector_size_

dev=loop-file
bootcode_size=446

# Create the test file
dd if=/dev/zero of=$dev bs=1M count=4 || fail=1

# Create msdos label
parted -s $dev mklabel msdos > out 2>&1 || fail=1
compare /dev/null out || fail=1 # Expect no output

# Create extended partition
parted -s $dev mkpart extended 2048s 8191s > out 2>&1 || fail=1
compare /dev/null out || fail=1 # Expect no output

# Create logical partition
parted -s $dev mkpart logical 4096s 8191s > out 2>&1 || fail=1
compare /dev/null out || fail=1 # Expect no output

# Install fake bootcode
printf %0${bootcode_size}d 0 > in || fail=1
dd if=in of=$dev bs=1c seek=1M count=$bootcode_size \
  conv=notrunc > /dev/null 2>&1 || fail=1

# Save fake bootcode for later comparison
dd if=$dev of=before bs=1 skip=1M count=$bootcode_size || fail=1

# Do something to the label
parted -s $dev rm 5 > out 2>&1 || fail=1
compare /dev/null out || fail=1 # Expect no output

# Extract the bootcode for comparison
dd if=$dev of=after bs=1 skip=1M count=$bootcode_size || fail=1

# Expect bootcode has not changed
compare before after || fail=1

Exit $fail
