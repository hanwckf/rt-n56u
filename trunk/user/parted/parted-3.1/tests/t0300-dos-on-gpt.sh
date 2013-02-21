#!/bin/sh
# avoid segfault creating a dos PT on top of a gpt one

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

PARTED_SECTOR_SIZE=4096
export PARTED_SECTOR_SIZE

dev=loop-file
# create a backing file large enough for a GPT partition table
dd if=/dev/null of=$dev seek=4001 2> /dev/null || framework_failure

# create a GPT partition table
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# create a DOS partition table on top of it
parted -s $dev mklabel msdos > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

Exit $fail
