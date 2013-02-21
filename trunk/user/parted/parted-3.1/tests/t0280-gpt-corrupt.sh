#!/bin/sh
# corrupt a GPT table; ensure parted takes notice

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

dev=loop-file

ss=$sector_size_
n_sectors=5000

dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1

# create gpt label
parted -s $dev mklabel gpt > empty 2>&1 || fail=1
compare /dev/null empty || fail=1

# print the empty table
parted -m -s $dev unit s print > t 2>&1 || fail=1
sed "s,.*/$dev:,$dev:," t > out || fail=1

# check for expected output
printf "BYT;\n$dev:${n_sectors}s:file:$sector_size_:$sector_size_:gpt::;\n" \
  > exp || fail=1
compare exp out || fail=1

# create a partition
parted -s $dev mkpart sw linux-swap 2048s 4095s > empty 2>&1 || fail=1
compare /dev/null empty || fail=1

# We're going to change the name of the first partition,
# thus invalidating the PartitionEntryArrayCRC32 checksum.
orig_byte=$(gpt_corrupt_primary_table_ $dev $ss) || fail=1

# printing the table must succeed, but with a scary diagnostic.
parted -s $dev print > err 2>&1 || fail=1
grep Error: err > k && mv k err || fail=1

# check for expected diagnostic
echo 'Error: The primary GPT table is corrupt, but the backup appears OK,' \
    'so that will be used.' > exp || fail=1
compare exp err || fail=1

# ----------------------------------------------------------
# Now, restore things, and corrupt the MyLBA in the alternate GUID table.

# Restore original byte
gpt_restore_primary_table_ $dev $ss "$orig_byte" || fail=1

# print the table
parted -s $dev print > out 2> err || fail=1
compare /dev/null err || fail=1

# The MyLBA member of the alternate table is in the last sector,
# $n_sectors, 8-byte field starting at offset 24.
alt_my_lba_offset=$(expr $n_sectors \* $ss - $ss + 24)
# get the first byte of MyLBA
byte=$(peek_ $dev $alt_my_lba_offset) || fail=1

# Perturb it.
test x"$byte" = xA && new_byte=B || new_byte=A

# Replace good byte with the bad one.
poke_ $dev $alt_my_lba_offset "$new_byte" || fail=1

# attempting to set partition name must print a diagnostic
parted -m -s $dev name 1 foo > err 2>&1 || fail=1

# check for expected diagnostic
echo 'Error: The backup GPT table is corrupt, but the primary appears OK,' \
    'so that will be used.' > exp || fail=1
compare exp err || fail=1

# corruption is fixed; printing the table now elicits no diagnostic
parted -m -s $dev u s print > out 2>&1 || fail=1

# check for expected output
printf "BYT;\nfile\n1:2048s:4095s:2048s::foo:;\n" > exp || fail=1
sed "s/.*gpt::;/file/" out > k && mv k out || fail=1
compare exp out || fail=1

Exit $fail
