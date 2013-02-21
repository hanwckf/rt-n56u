#!/bin/sh
# partprobe must not examine more than 16 partitions

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
require_erasable_
require_root_
require_dvhtool_

dev=$DEVICE_TO_ERASE

# setup: create a DVH partition table on $dev"
dd if=/dev/zero of=$dev bs=512 count=1 seek=10000 || fail=1
parted -s $dev mklabel dvh || fail=1

# setup: use dvhtool to create a 17th (invalid?) partition"
dd if=/dev/zero of=d bs=1 count=4k || fail=1
dvhtool -d $dev --unix-to-vh d data || fail=1

# Here's sample output from the parted...print command below:
# BYT;
# /dev/sdd:128880s:scsi:512:512:dvh: Flash Disk:;
# 9:0s:4095s:4096s:::;
# 17:4s:11s:8s::data:;

# ensure that dvhtool did what we want"
parted -m -s $dev unit s print > out 2>&1 || fail=1
grep "^17:.*::data:;\$" out || fail=1

# Parted 1.8.9 and earlier would mistakenly try to access partition #17.
# ensure that partprobe succeeds and produces no output"
partprobe -s $dev > out 2>err || fail=1
compare /dev/null err || fail=1
echo "$dev: dvh partitions 9 <17>" > exp || fail=1
compare exp out || fail=1

Exit $fail
