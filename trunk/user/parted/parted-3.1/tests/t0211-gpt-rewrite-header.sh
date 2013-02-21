#!/bin/sh
# exercise parted's gpt-rewriting code
# When it encounters a GPT device with backup not at the end,
# parted (without -s) offers to correct that by moving the backup
# header to the end of the device.  Before parted-3.1, when it attempted
# to do that, starting with a 9-PTE array, it would render the result invalid.

# Copyright (C) 2012 Free Software Foundation, Inc.

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

ns=100         # Initial number of sectors.
ns2=$((ns+64)) # Some larger number of sectors.
dev=loop-file
# create a file large enough to hold a GPT partition table
dd if=/dev/null of=$dev bs=$ss seek=$ns || framework_failure

# create a GPT partition table with 9 partitions in a standard 128-entry table.
parted -a min -s $dev mklabel gpt \
    mkpart p1 34s 34s \
    mkpart p2 35s 35s \
    mkpart p3 36s 36s \
    mkpart p4 37s 37s \
    mkpart p5 38s 38s \
    mkpart p6 39s 39s \
    mkpart p7 40s 40s \
    mkpart p8 41s 41s \
    mkpart p9 42s 42s \
  > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

# Adjust the GPT table to have only 9 PTEs.
gpt-header-munge --sector-size=$ss --n=9 $dev || fail=1

# Extend size of the device.
dd if=/dev/null of=$dev bs=$ss seek=$ns2 || fail=1

printf '%s\n' f f > in || fail=1

# This would always succeed, even before.
parted ---pretend-input-tty $dev u s p < in || fail=1

printf '%s\n' \
    'BYT;' \
    "...:${ns2}s:file:$ss:$ss:gpt::;" \
    '1:34s:34s:1s::p1:;' \
    '2:35s:35s:1s::p2:;' \
    '3:36s:36s:1s::p3:;' \
    '4:37s:37s:1s::p4:;' \
    '5:38s:38s:1s::p5:;' \
    '6:39s:39s:1s::p6:;' \
    '7:40s:40s:1s::p7:;' \
    '8:41s:41s:1s::p8:;' \
    '9:42s:42s:1s::p9:;' \
  > exp || fail=1

# Before parted-3.1, this would fail, reporting that both tables are corrupt.
parted -m -s $dev u s p > out 2>&1 || fail=1
sed '2s/^[^:]*:/...:/' out > k && mv k out
compare exp out || fail=1

Exit $fail
