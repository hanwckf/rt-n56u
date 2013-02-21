#!/bin/sh
# Confirm that specifying 1 unit snaps to the correct value

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

ss=$sector_size_
n_sectors=3000
dev=dev-file

# Create an example of what the result should look like
# start should be at 1 sector.
dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1
parted --align=none -s $dev mklabel msdos mkpart pri 1s $((1000*1000))B \
    > err 2>&1 || fail=1
compare /dev/null err || fail=1
parted -m -s $dev u s p > exp || fail=1

rm $dev
dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1
parted --align=none -s $dev mklabel msdos mkpart pri 0 1MB \
    > err 2>&1 || fail=1
compare /dev/null err || fail=1
parted -m -s $dev u s p > out || fail=1

compare exp out || fail=1

Exit $fail
