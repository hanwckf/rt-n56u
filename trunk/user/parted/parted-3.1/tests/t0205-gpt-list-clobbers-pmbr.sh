#!/bin/sh
# Ensure that printing a GPT partition table does not modify the pMBR.
# Due to a bug in parted-2.1, "parted /dev/... print" would do just that.
# Not a problem for most, but if you have a hybrid, e.g., gptsync'd
# GPT/MBR table, merely listing the table with Parted-2.1 would clobber
# the MBR part.

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

ss=$sector_size_
n_sectors=400
dev=dev-file

dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1
parted -s $dev mklabel gpt                     || fail=1
parted -s $dev mkpart p1 101s 150s             || fail=1
parted -s $dev mkpart p2 151s 200s             || fail=1
parted -s $dev mkpart p3 201s 250s             || fail=1

parted -m -s $dev u s p                        || fail=1

# Write non-NUL bytes all over the MBR, so we're likely to see any change.
# However, be careful to leave the type of the first partition, 0xEE,
# as well as the final two magic bytes.
printf '%0450d\xee%059d\x55\xaa' 0 0 | dd of=$dev count=1 conv=notrunc || fail=1

dd if=$dev of=before count=1 || fail=1

chmod a-w $dev
parted -m -s $dev u s p || fail=1

dd if=$dev of=after count=1 || fail=1

cmp before after || fail=1

Exit $fail
