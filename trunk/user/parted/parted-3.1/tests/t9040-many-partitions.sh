#!/bin/sh
# Ensure that creating many partitions works.

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
partition_sectors=1    # sectors per partition
n_partitions=128       # how many partitions to create
start=34               # start sector for the first partition
gpt_slop=34            # sectors at end of disk reserved for GPT

n_sectors=$(($start + n_partitions * partition_sectors + gpt_slop))

sectors_per_MiB=$((1024 * 1024 / ss))
n_MiB=$(((n_sectors + sectors_per_MiB - 1) / sectors_per_MiB))
# create memory-backed device
scsi_debug_setup_ sector_size=$ss dev_size_mb=$n_MiB > dev-name ||
  skip_ 'failed to create scsi_debug device'
scsi_dev=$(cat dev-name)

n=$((n_MiB * sectors_per_MiB))
printf "BYT;\n$scsi_dev:${n}s:scsi:$ss:$ss:gpt:Linux scsi_debug:;\n" \
  > exp || fail=1

cmd=
i=1
while :; do
    s=$((start + i - 1))
    e=$((s + partition_sectors - 1))
    cmd="$cmd mkpart p$i ${s}s ${e}s"
    test $i = $n_partitions && break; i=$((i+1))
done

# Time the actual command:
t0=$(date +%s.%N)
parted -m -a min -s $scsi_dev mklabel gpt $cmd u s p > out 2>&1 || fail=1
t_final=$(date +%s.%N)

i=1
while :; do
    s=$((start + i - 1))
    e=$((s + partition_sectors - 1))
    printf "$i:${s}s:${e}s:${partition_sectors}s::p$i:;\n" >> exp
    test $i = $n_partitions && break; i=$((i+1))
done

# Fail the test if it takes too long.
# On Fedora 16, this takes about 10 seconds for me.
# With Fedora-12-era kernels, it typically took more than 150 seconds.
$AWK "BEGIN {d = $t_final - $t0; n = $n_partitions; st = 60 < d;"\
' printf "created %d partitions in %.2f seconds\n", n, d; exit st }' /dev/null \
    || fail=1

compare exp out || fail=1

Exit $fail
