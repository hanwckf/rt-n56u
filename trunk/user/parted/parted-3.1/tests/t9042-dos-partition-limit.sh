#!/bin/sh
# Ensure that parted enforces msdos partition limit

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

require_root_
require_scsi_debug_module_

grep '^#define USE_BLKID 1' "$CONFIG_HEADER" > /dev/null ||
  skip_ 'this system lacks a new-enough libblkid'

ss=$sector_size_
partition_sectors=256  # sectors per partition
n_partitions=61        # how many partitions to create
start=2048             # start sector for the first partition

n_sectors=$(($start + n_partitions * partition_sectors))

sectors_per_MiB=$((1024 * 1024 / ss))
n_MiB=$(((n_sectors + sectors_per_MiB - 1) / sectors_per_MiB))
# create memory-backed device
scsi_debug_setup_ sector_size=$ss dev_size_mb=$n_MiB > dev-name ||
  skip_ 'failed to create scsi_debug device'
scsi_dev=$(cat dev-name)

n=$((n_MiB * sectors_per_MiB))
printf '%s\n' "BYT;" \
    "$scsi_dev:${n}s:scsi:$ss:$ss:msdos:Linux scsi_debug:;" \
    "1:$((start-2))s:$((n-1))s:$((n-start+2))s:::lba;" \
  > exp || fail=1

parted -s $scsi_dev mklabel msdos || fail=1
parted -s -a min $scsi_dev mkpart extended $((start-2))s 100% || fail=1

i=1
while :; do
    end=$((start + partition_sectors - 2))
    parted -s -a min $scsi_dev mkpart logical ${start}s ${end}s || fail=1
    printf "$((i+4)):${start}s:${end}s:$((end-start+1))s:::;\n" >> exp
    test $i = $((n_partitions - 1)) && break
    start=$((start + partition_sectors))
    i=$((i+1))
done

parted -m -s $scsi_dev u s p > out || fail=1
compare exp out || fail=1

start=$((start + partition_sectors))
end=$((start + partition_sectors - 2))

#try one more partition than allowed, make sure it fails

parted -s -a min $scsi_dev mkpart logical ${start}s ${end}s > out 2>&1
cat <<EOF > exp
Error: cannot create any more partitions
EOF
compare exp out || fail=1

Exit $fail
