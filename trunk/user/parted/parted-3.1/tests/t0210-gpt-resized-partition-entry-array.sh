#!/bin/sh
# exercise GPT handling of n_partition_array_entries != 128

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

. "${srcdir=.}/init.sh"; path_prepend_ ../parted $srcdir
require_perl_digest_crc_

ss=$sector_size_

N=2M
dev=loop-file
# create a file large enough to hold a GPT partition table
dd if=/dev/null of=$dev bs=1 seek=$N || framework_failure

# create a GPT partition table
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

gpt-header-munge --sector-size=$ss --n=9 $dev || fail=1

parted -m -s $dev u s p > out 2>&1 || fail=1
sed 's/.*:gpt:/:gpt:/' out > k && mv k out
printf 'BYT;\n:gpt::;\n' > exp || fail=1
compare exp out || fail=1

Exit $fail
