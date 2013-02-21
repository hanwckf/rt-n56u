#!/bin/sh
# the most basic 'print' test
# Copyright (C) 2007, 2009-2012 Free Software Foundation, Inc.

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

ss=$sector_size_
dev=loop-file

{
  cat <<EOF
Model:  (file)
Disk .../$dev: 8s
Sector size (logical/physical): ${ss}B/${ss}B
Partition Table: msdos
Disk Flags:

Number  Start  End  Size  Type  File system  Flags

EOF
} > exp || framework_failure

# Using msdos_magic='\x55\xaa' looks nicer, but isn't portable.
# dash's builtin printf doesn't recognize such \xHH hexadecimal escapes.
msdos_magic='\125\252'

# The extra 3KB+ zero bytes at the end are to avoid triggering a failure
# on linux-2.6.8 that's probably related to opening with O_DIRECT.
# Note that the minimum number of appended zero bytes required to avoid
# the failure was 3465.  Here, we append a little more to make the resulting
# file have a total size of exactly 8 sectors.
# setup: create the most basic partition table, manually
{ dd if=/dev/zero  bs=510 count=1; printf "$msdos_magic"
  dd if=/dev/zero bs=$(expr 8 '*' $ss - 510) count=1; } > $dev || fail=1

# print the empty table
parted -s $dev unit s print >out 2>&1 || fail=1

# prepare actual and expected output
sed 's/ $//' out > k && mv k out || fail=1 # Remove trailing blank.
mv out o2 && sed "s,^Disk .*/$dev:,Disk .../$dev:," o2 > out || fail=1

# check for expected output
compare exp out || fail=1

Exit $fail
