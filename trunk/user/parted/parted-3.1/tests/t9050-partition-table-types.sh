#!/bin/sh
# Ensure that any combination of partition table types works.
# I.e., write a partition table of type T, and then overwrite it
# with one of type V, for every permutation of T and V.

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

not_yet='
aix
'

types='
bsd
msdos
dvh
gpt
loop
mac
pc98
sun
mkswap
'

dd if=/dev/null of=f bs=1 seek=30M || framework_failure_

for i in $types; do
  for j in $types; do
    echo $i:$j
    case $i in mkswap) mkswap f || fail=1;;
      *) parted -s f mklabel $i || fail=1;; esac
    case $j in mkswap) continue;; esac
    parted -s f mklabel $j || fail=1
  done
done

Exit $fail
