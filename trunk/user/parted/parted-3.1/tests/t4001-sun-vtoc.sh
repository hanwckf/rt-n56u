#!/bin/sh
# Ensure that Sun VTOC is properly initialized.

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

# Written by Karel Zak <kzak@redhat.com>

. "${srcdir=.}/init.sh"; path_prepend_ ../parted

N=2M
dev=loop-file
# create a file to simulate the underlying device
dd if=/dev/null of=$dev bs=1 seek=$N || framework_failure

# label the test disk
parted -s $dev mklabel sun > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# extract version
od -t x1 -An -j128 -N4 $dev > out || fail=1
echo " 00 00 00 01" > exp || fail=1
# expect it to be 00 00 00 01, not 00 00 00 00
compare exp out || fail=1

# extract nparts
od -t x1 -An -j140 -N2 $dev > out || fail=1
echo " 00 08" > exp || fail=1

# expect it to be 00 08, not 00 00
compare exp out || fail=1

# extract sanity magic
od -t x1 -An -j188 -N4 $dev > out || fail=1
echo " 60 0d de ee" > exp
# expect it to be 60 0d de ee, not 00 00 00 00
compare exp out || fail=1

Exit $fail
