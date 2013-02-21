#!/bin/sh
# parted before 3.1 could abort for a pathologically small device with
# a valid primary GPT header but no room for the backup header.

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

# truncate it to 34 sectors.
for i in 33 34 35 67 68 69 101 102 103; do
  dd if=$dev of=bad count=$i

  # Print the partition table.  Before, this would evoke a failed assertion.
  printf 'i\no\n' > in
  parted ---pretend-input-tty bad u s p < in > out 2> err || { fail=1; cat err; }
  # don't bother comparing stdout
done

Exit $fail
