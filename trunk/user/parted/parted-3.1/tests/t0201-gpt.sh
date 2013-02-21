#!/bin/sh
# avoid failed assertion when creating a GPT on top of an old one for a
# larger device

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
# create a backing file large enough for a GPT partition table
dd if=/dev/null of=$dev seek=4001 2> /dev/null || fail=1

# create a GPT partition table
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# shrink the backing file
dd if=/dev/null of=$dev seek=4000 2> /dev/null || fail=1

# create a new GPT table on top of the shrunken backing file
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

Exit $fail
