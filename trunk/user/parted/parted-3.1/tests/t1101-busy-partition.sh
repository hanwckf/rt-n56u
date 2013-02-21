#!/bin/sh
# test for Debian bug #582818 (http://bugs.debian.org/582818); forbid
# the removal of a mounted partition.

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
test "$VERBOSE" = yes && parted --version

require_root_
require_scsi_debug_module_

# create memory-backed device
scsi_debug_setup_ dev_size_mb=80 > dev-name ||
  skip_ 'failed to create scsi_debug device'
dev=$(cat dev-name)

cat <<EOF > exp-error || framework_failure
Error: Partition ${dev}2 is being used. You must unmount it before you modify it with Parted.
EOF

parted -s "$dev" mklabel msdos > out 2>&1 || fail=1

# expect no output
compare /dev/null out || fail=1

parted -s "$dev" mkpart primary fat32 1 40 > out 2>&1 || fail=1
compare /dev/null out || fail=1

parted -s "$dev" mkpart primary fat32 40 80 > out 2>&1 || fail=1
compare /dev/null out || fail=1

# wait for new partition device to appear
wait_for_dev_to_appear_ ${dev}2 || fail_ ${dev}2 did not appear

mkfs.vfat -F 32 ${dev}2 || skip_ mkfs.vfat failed

# be sure to unmount upon interrupt, failure, etc.
cleanup_fn_() { umount "${dev}2" > /dev/null 2>&1; }

mount_point=$(pwd)/mnt

mkdir $mount_point || fail=1
mount "${dev}2" "$mount_point" || fail=1

# Removal of unmounted partition must succeed.
parted -s "$dev" rm 1 > out 2>&1 || fail=1

# Removal of mounted partition must fail.
parted -s "$dev" rm 2 > out 2>&1 && fail=1

# expect error
compare exp-error out || fail=1

Exit $fail
