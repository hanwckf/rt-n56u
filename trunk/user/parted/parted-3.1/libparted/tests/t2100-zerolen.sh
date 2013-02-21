#!/bin/sh
# run the zerolen unit tests in a directory supporting O_DIRECT

# Copyright (C) 2007-2012 Free Software Foundation, Inc.

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

. "${abs_top_srcdir=../..}/tests/init.sh"; path_prepend_ .
. $abs_top_srcdir/tests/t-lib-helpers.sh
# Need root privileges to create a device-mapper device.
require_root_
device_mapper_required_

init_root_dir_

# This test only makes sense on Linux.
test "$(uname -s)" = Linux \
  || skip_ "not on Linux"

test "x$DYNAMIC_LOADING" = xyes \
  || skip_ "no dynamic loading support"

test "x$ENABLE_DEVICE_MAPPER" = xyes \
  || skip_ "no device-mapper support"

# Device map name - should be random to not conflict with existing ones on
# the system
linear_=plinear-$$

cleanup_fn_()
{
  # 'dmsetup remove' may fail because udev is still processing the device.
  # Try it repeatedly for 2s.
  i=0
  incr=1
  while :; do
    dmsetup remove $linear_ > /dev/null 2>&1 && break
    sleep .1 2>/dev/null || { sleep 1; incr=10; }
    i=$(expr $i + $incr); test $i = 20 && break
  done
  if test $i = 20; then
    dmsetup remove $linear_
  fi

  test -n "$d1" && losetup -d "$d1"
  rm -f "$f1"
}

f1=$(pwd)/1
d1=$(loop_setup_ "$f1") \
  || skip_ "is this partition mounted with 'nodev'?"

echo "0 1024 linear $d1 0" | dmsetup create "$linear_" \
  || skip_ "unable to create dm device"

wait_for_dev_to_appear_ "/dev/mapper/$linear_" \
  || skip_ "dm device did not appear"

zerolen /dev/mapper/$linear_ || fail=1

Exit $fail
