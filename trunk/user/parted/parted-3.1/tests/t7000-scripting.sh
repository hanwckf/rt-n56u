#!/bin/sh
# Make sure the scripting option works (-s) properly.

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
ss=$sector_size_
N=100 # number of sectors

: ${abs_top_builddir=$(cd ../..; pwd)}
: ${CONFIG_HEADER="$abs_top_builddir/lib/config.h"}

config_h=$abs_top_srcdir
grep '^#define HAVE_LIBREADLINE 1' $CONFIG_HEADER > /dev/null \
  || skip_ "configured without readline support"

# The failure messages.
cat << EOF > err.expected || fail=1
Error: You requested a partition from 512B to 50.7kB (sectors 0..0).
The closest location we can manage is 17.4kB to 33.8kB (sectors 0..0).
EOF

normalize_part_diag_ err.expected || fail=1

{ emit_superuser_warning
  sed s/Error/Warning/ err.expected
  printf 'Is this still acceptable to you?\nYes/No?'; } >> errI || fail=1

for mkpart in mkpart; do

  # Test for mkpart in scripting mode
  # Create the test file
  dd if=/dev/zero of=testfile bs=${ss}c count=$N 2> /dev/null || fail=1

  # Test the scripting mode of $mkpart.
  parted -s testfile -- mklabel gpt "$mkpart" p-name ext3 1s -1s > out 2>&1
  test $? = 1 || fail=1

  # Compare the real error and the expected one
  normalize_part_diag_ out || fail=1
  compare err.expected out || fail=1

  # Test mkpart interactive mode.
  # Create the test file
  rm -f testfile
  dd if=/dev/zero of=testfile bs=${ss}c count=$N 2> /dev/null || fail=1
  # Test the interactive mode of $mkpart
  echo n | \
    parted ---pretend-input-tty testfile \
      "mklabel gpt '$mkpart' p-name ext3 1s -1s" > out 2>&1 && fail=1

  # We have to format the output before comparing.
  # normalize the actual output
  printf x >> out || fail=1
  sed "s,   *,,g;s, x$,,;/ n$/ {N;s, n\nx,,}" out > o2 && mv -f o2 out \
      || fail=1
  normalize_part_diag_ out || fail=1

  # Compare the real error and the expected one
  compare out errI || fail=1

done

Exit $fail
