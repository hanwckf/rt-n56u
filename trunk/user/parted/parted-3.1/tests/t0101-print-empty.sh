#!/bin/sh
# test 'parted $dev print' on empty device (without label)
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

ss=$sector_size_
dev=loop-file

{
  cat <<EOF
Error: .../$dev: unrecognised disk label
Model:  (file)
Disk .../$dev: 8s
Sector size (logical/physical): ${ss}B/${ss}B
Partition Table: unknown
Disk Flags:
EOF
} > exp || framework_failure

# create 'empty' device
dd if=/dev/zero of=$dev bs=$(expr 8 '*' $ss) count=1 >/dev/null 2>&1 || fail=1

# print the empty table; expect nonzero exit status
parted -s $dev unit s print >out 2>&1 && fail=1

# prepare actual and expected output
sed 's/ $//' out > k && mv k out || fail=1 # Remove trailing blank.
mv out o2 && sed "s,^Disk .*/$dev:,Disk .../$dev:,; \
                  s,^Error: .*/$dev:,Error: .../$dev:," o2 > out || fail=1

# check for expected output
compare exp out || fail=1

Exit $fail
