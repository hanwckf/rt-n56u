#!/bin/sh
# test bios_grub flag in gpt labels

# Copyright (C) 2007-2012 Free Software Foundation, Inc.

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
N=4200 # number of sectors

part_sectors=2048
start_sector=2048
end_sector=$(expr $start_sector + $part_sectors - 1)

# setup: reasonable params
test $end_sector -lt $N || fail=1

# setup: create zeroed device
dd if=/dev/zero of=$dev bs=${ss}c count=$N 2> /dev/null || fail=1

# create gpt label
parted -s $dev mklabel gpt > empty 2>&1 || fail=1

# ensure there was no output
compare /dev/null empty || fail=1

# print the table (before adding a partition)
parted -m -s $dev unit s print > t 2>&1 || fail=1
sed "s,.*/$dev:,$dev:," t > out || fail=1

# check for expected output
printf "BYT;\n$dev:${N}s:file:$ss:$ss:gpt::;\n" > exp || fail=1
compare exp out || fail=1

# add a partition
parted -s $dev u s mkpart name1 ${start_sector} ${end_sector} >out 2>&1 \
  || fail=1

# print the table before modification
parted -m -s $dev unit s print > t 2>&1 || fail=1
sed "s,.*/$dev:,$dev:," t >> out || fail=1

# set the new bios_grub attribute
parted -m -s $dev set 1 bios_grub on || fail=1

# print the table after modification
parted -m -s $dev unit s print > t 2>&1 || fail=1
sed "s,.*/$dev:,$dev:," t >> out || fail=1

gen_exp()
{
  cat <<EOF
BYT;
$dev:${N}s:file:$ss:$ss:gpt::;
1:${start_sector}s:${end_sector}s:${part_sectors}s::name1:;
BYT;
$dev:${N}s:file:$ss:$ss:gpt::;
1:${start_sector}s:${end_sector}s:${part_sectors}s::name1:bios_grub;
EOF
}

# check for expected output
gen_exp > exp || fail=1
compare exp out || fail=1

Exit $fail
