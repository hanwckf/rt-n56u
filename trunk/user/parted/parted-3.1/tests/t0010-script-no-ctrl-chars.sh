#!/bin/sh
# Ensure that printing with -s outputs no readline control chars

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
n_sectors=5000
dev=loop-file

dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1

parted -s $dev mklabel msdos > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# print partition table in --script mode
TERM=xterm parted -m -s $dev u s p > out 2>&1 || fail=1

sed "s,.*/$dev:,$dev:," out > k && mv k out || fail=1
printf "BYT;\n$dev:${n_sectors}s:file:$ss:$ss:msdos::;\n" > exp || fail=1

compare exp out || fail=1

Exit $fail
