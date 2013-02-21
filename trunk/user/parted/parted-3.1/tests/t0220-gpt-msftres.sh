#!/bin/sh
# gpt default "flag" for a partition must not be msftres

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

# FIXME: should be able to use "ufs" here, too, but that doesn't work.
fs_types='
ext2
fat16
fat32
hfs
hfs+
hfsx
linux-swap
NTFS
reiserfs
'

start=2048
part_size=2048
n_types=$(echo "$fs_types"|wc -w)

# Create a "disk" with enough room for one partition per FS type,
# and the overhead required for a GPT partition table.
# 32 is the number of 512-byte sectors required to accommodate the
# minimum size of the secondary GPT header at the end of the disk.
n_sectors=$(expr $start + $n_types \* $part_size + 1 + 32)

# create a test file large enough for one partition per FS type
dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1

# create a gpt partition table
parted -s $dev mklabel gpt > out 2>&1 || fail=1
# expect no output
compare /dev/null out || fail=1

printf "BYT;\n$dev:${n_sectors}s:file:$ss:$ss:gpt::;\n" > exp
i=1
for type in $fs_types; do
  end=$(expr $start + $part_size - 1)
  echo "$i:${start}s:${end}s:${part_size}s::$type:;" >> exp || fail=1
  parted -s $dev mkpart p-name $type ${start}s ${end}s > err 2>&1 || fail=1
  compare /dev/null err || fail=1
  parted -s $dev name $i $type > err 2>&1 || fail=1
  compare /dev/null err || fail=1
  start=$(expr $end + 1)
  i=$(expr $i + 1)
done

# print partition table
parted -m -s $dev u s p > out 2>&1 || fail=1

sed "s,.*/$dev:,$dev:," out > k && mv k out && ok=1 || ok=0
# match against expected output
test $ok = 1 && { compare exp out || fail=1; }

Exit $fail
