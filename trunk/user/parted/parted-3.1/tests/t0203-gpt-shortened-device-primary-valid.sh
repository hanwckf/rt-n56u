#!/bin/sh
# Demonstrate that placing a valid gpt-labeled image on a shorter device
# does not invalidate the primary GPT header.

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

. "${srcdir=.}/init.sh"; path_prepend_ ../parted
ss=$sector_size_

dev=dev-file
dd if=/dev/null of=$dev bs=$ss seek=100 || fail=1
parted -s -- $dev mklabel gpt || fail=1

# Chop off the last two sectors.
dd if=/dev/null of=$dev bs=$ss seek=98 || fail=1
printf 'ignore\nok\n' > in
parted -m ---pretend-input-tty $dev u s p < in > out 2> err || fail=1

# Remove abs name of $dev_file.
sed "s, [^ ]*/$dev, $dev," err > k && mv k err || fail=1
# Compare only the last line, to avoid control chars of interactive mode.
tail -1 out > k && mv k out || fail=1
sed "s,.*/$dev:,$dev:," out > k && mv k out || fail=1

emit_superuser_warning > err.exp || fail=1
cat <<EOF >> err.exp || fail=1
Error: end of file while reading $dev
Error: The backup GPT table is corrupt, but the primary appears OK, so that will be used.
EOF

echo "$dev:98s:file:$ss:$ss:gpt::;" > out.exp || fail=1

compare err.exp err || fail=1
compare out.exp out || fail=1

Exit $fail
