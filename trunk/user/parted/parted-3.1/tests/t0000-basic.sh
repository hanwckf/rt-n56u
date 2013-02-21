#!/bin/sh
# Ensure that a simple command using -s succeeds with no prompt

# Copyright (C) 2007, 2009-2012 Free Software Foundation, Inc.

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

parted --version || fail_ 'You have not built parted yet.'

# FIXME: is id -u portable enough?
uid=`id -u` || uid=1

# create a file of size N bytes
N=1M
dev=loop-file

# create the test file
dd if=/dev/null of=$dev bs=1 seek=$N 2> /dev/null || fail=1

# run parted -s FILE mklabel msdos
parted -s $dev mklabel msdos > out 2>&1 || fail=1
compare /dev/null out || fail=1

# ----------------------------------------------
# Now, ensure that a simple mklabel command succeeds.
# Since there's no -s option, there are prompts -- sometimes.

# erase the left-over label
dd if=/dev/zero of=$dev bs=4K count=1 conv=notrunc 2> /dev/null || fail=1

# First iteration works with no prompting, since there is no preexisting label.
# run parted mklabel (without -s) on a blank disk
parted $dev mklabel msdos > out 2>&1 || fail=1

# create expected output file
emit_superuser_warning > exp || fail=1

# check its "interactive" output
compare exp out || fail=1

# create interactive input
printf 'y\n' > in || fail=1

# Now that there's a label, rerunning the same command is interactive.
# rerun that same command, but now with a preexisting label
parted ---pretend-input-tty $dev mklabel msdos < in > out 2>&1 || fail=1

# Transform the actual output, to avoid spurious differences when
# $PWD contains a symlink-to-dir.  Also, remove the ^M      ...^M bogosity.
# normalize the actual output
mv out o2 && sed -e "s,on /.*/$dev,on DEVICE,;s,   *,,g;s, $,," \
                      -e "s,^.*/lt-parted: ,parted: ," o2 > out

# Create expected output file.
{ emit_superuser_warning > exp; } || fail=1
cat <<EOF >> exp || fail=1
Warning: The existing disk label on DEVICE will be destroyed and all\
 data on this disk will be lost. Do you want to continue?
Yes/No? y
EOF

# check its output -- slightly different here, due to prompts
compare exp out || fail=1

Exit $fail
