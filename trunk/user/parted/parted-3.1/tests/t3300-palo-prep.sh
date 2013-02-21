#!/bin/sh
# Ensure that palo and prep types work properly.

# Copyright (C) 2010-2012 Free Software Foundation, Inc.

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

cat > exp <<EOF || framework_failure
1:2048s:4095s:2048s:::palo;
1:2048s:4095s:2048s:::prep;
1:2048s:4095s:2048s:::palo;
EOF

dev=dev-file

n_sectors=5000
dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1

parted -m -s $dev mklabel msdos \
  mkpart pri ext2 $((1*2048))s $((2*2048-1))s \
  set 1 palo on u s print \
  set 1 prep on u s print \
  set 1 palo on u s print \
    > out 2> err || fail=1

grep -E '^1:2048s:4095s:2048s:::p...;$' out > k; mv k out

compare exp out || fail=1

Exit $fail
