#!/bin/sh
# operate on a very small (1-sector) "disk"

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
dev=loop-file

for opt in '' -s; do

  dd if=/dev/null of=$dev bs=1 seek=$ss || framework_failure

  # create an msdos partition table:
  # Before parted-2.1, without -s, this would fail with a bogus diagnostic:
  # Error: Success during read on .../tests/loop-file
  # Retry/Ignore/Cancel? ^C
  parted $opt $dev mklabel msdos ---pretend-input-tty </dev/null > out 2>&1 \
      || fail=1
  # expect no output
  sed 's/.*WARNING: You are not superuser.*//;/^$/d' out > k && mv k out \
      || fail=1
  # When run as root, there are just curses-related control chars. Remove them.
  sed 's/^.\{1,12\}$//;/^$/d' out > k && mv k out \
      || fail=1
  compare /dev/null out || fail=1

  parted -s $dev p || fail=1
  rm -f $dev

done

Exit $fail
