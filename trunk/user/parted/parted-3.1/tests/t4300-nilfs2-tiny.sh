#!/bin/sh
# Trigger a nilfs2-related bug.

# Copyright (C) 2011-2012 Free Software Foundation, Inc.

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

n_sectors=200
dev=dev-file
dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || framework_failure_

# Create a tiny, 7-sector partition.
parted -s $dev mklabel gpt mkpart p1 64s 70s || framework_failure_

# This used to make parted abort.
parted -s $dev u s p || fail=1

Exit $fail
