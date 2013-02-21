#!/bin/sh
# gpt: create many partitions
# Before parted-3.1, this would provoke an invalid free.

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

ss=$sector_size_

ns=300
n_partitions=128
dev=dev-file
start_sector=34

# create a file large enough to hold a GPT partition table
dd if=/dev/null of=$dev bs=$ss seek=$ns || framework_failure

cmd=
i=1
while :; do
  s=$((start_sector + i - 1))
  cmd="$cmd mkpart p$i ${s}s ${s}s"
  test $i = $n_partitions && break; i=$((i+1))
done
parted -m -a min -s $dev mklabel gpt $cmd u s p > out 2>&1 || fail=1

nl='
'
exp=$(printf '%s\n' 'BYT;' "...:${ns}s:file:$ss:$ss:gpt::;")"$nl"

i=1
while :; do
  s=$((start_sector + i - 1))
  exp="$exp$i:${s}s:${s}s:1s::p$i:;$nl"
  test $i = $n_partitions && break; i=$((i+1))
done
printf %s "$exp" > exp || fail=1

sed '2s/^[^:]*:/...:/' out > k && mv k out
compare exp out || fail=1

Exit $fail
