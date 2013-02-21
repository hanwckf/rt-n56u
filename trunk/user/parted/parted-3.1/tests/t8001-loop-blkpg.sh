#!/bin/sh
# Test support for partitions on loop devices

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

require_root_
require_udevadm_settle_

cleanup_fn_()
{
  test -n "$loopdev" \
    && { udevadm settle --timeout=3; losetup -d "$loopdev"; }
}

# If the loop module is loaded, unload it first
if lsmod | grep '^loop[[:space:]]'; then
    rmmod loop || fail=1
fi

# Insert loop module with max_part > 1
modprobe loop max_part=7 || fail=1

# Create backing file
dd if=/dev/zero of=backing_file bs=1M count=4 >/dev/null 2>&1 || fail=1

# Set up loop device on top of backing file
loopdev=$(losetup -f --show backing_file)
test -z "$loopdev" && fail=1

require_partitionable_loop_device_ $loopdev

# Expect this to succeed
parted -s "$loopdev" mklabel msdos > err 2>&1 || fail=1
compare /dev/null err || fail=1     # expect no output

# Create a partition
parted -s "$loopdev" mkpart primary 1M 2M > err 2>&1 || fail=1
compare /dev/null err || fail=1     # expect no output
udevadm settle --timeout=5 || fail=1

# Verify that the partition appeared in /proc/partitions
entry=$(basename "$loopdev"p1)
grep "$entry" /proc/partitions || { cat /proc/partitions; fail=1; }

# Remove the partition
parted -s "$loopdev" rm 1 > err 2>&1 || fail=1
compare /dev/null err || fail=1     # expect no output
udevadm settle --timeout=5 || fail=1

# Verify that the partition got removed from /proc/partitions
grep "$entry" /proc/partitions && fail=1

Exit $fail
