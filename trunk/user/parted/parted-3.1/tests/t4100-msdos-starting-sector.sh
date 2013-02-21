#!/bin/sh
# Consistency in msdos free space starting sector.

# Copyright (C) 2008-2012 Free Software Foundation, Inc.

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

######################################################################
# parted 1.8.8.1 and earlier was inconsistent when calculating the
# start sector for free space in msdos type lables.  parted was not
# consistent in the use of metadata padding for msdos labels.
######################################################################

N=4096 # number of sectors
dev=loop-file
# create a file to simulate the underlying device
dd if=/dev/zero of=$dev bs=${ss}c count=$N 2> /dev/null || fail=1

# label the test disk
parted -s $dev mklabel msdos > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# Test the output of print free with no partitions.
cat <<EOF > exp || fail=1
BYT;
path:${N}s:file:$ss:$ss:msdos::;
1:32s:4095s:4064s:free;
EOF

# create expected output file' 'test $fail = 0'

# display output of label without partitions
parted -m -s $dev unit s print free > out 2>&1 || fail=1

# check for expected output
sed "2s/^[^:]*:/path:/" < out > k; mv k out
compare exp out || fail=1

# Test the output of print free with one partition.
cat <<EOF > exp || fail=1
BYT;
path:${N}s:file:$ss:$ss:msdos::;
1:32s:2047s:2016s:free;
1:2048s:4095s:2048s:::;
EOF

# create a partition at the end of the label
parted -s $dev mkpart primary 2048s 4095s || fail=1

# display output of label with partition
parted -m -s $dev unit s print free > out 2>&1 || fail=1

# check for expected output
sed "2s/^[^:]*:/path:/" < out > k; mv k out
compare exp out || fail=1

Exit $fail
