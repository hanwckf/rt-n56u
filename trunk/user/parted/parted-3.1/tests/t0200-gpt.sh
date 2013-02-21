#!/bin/sh
# Ensure that printing a GPT partition table does not modify it.

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

N=2M
dev=loop-file
# create a file large enough to hold a GPT partition table
dd if=/dev/null of=$dev bs=1 seek=$N || framework_failure

# create a GPT partition table
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# save a copy of the original primary GPT table
dd if=$dev of=before count=1 skip=1 || fail=1

# extend the backing file by 1 byte
printf x >> $dev || fail=1

# use parted simply to print the partition table
parted -m -s $dev u s p > out 2> err || fail=1
# don't bother comparing stdout
# expect no stderr
compare /dev/null err || fail=1

# extract the primary GPT table again
dd if=$dev of=after count=1 skip=1 || fail=1

# compare partition tables (they had better be identical)
compare before after || fail=1

Exit $fail
