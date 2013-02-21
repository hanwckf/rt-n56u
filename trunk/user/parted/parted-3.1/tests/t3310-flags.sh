#!/bin/sh
# Exercise the exclusive, single-bit flags.

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
dev=dev-file

extract_flags()
{
  perl -nle '/^1:2048s:4095s:2048s::(?:P1)?:(.+);$/ and print $1' "$@"
}

for table_type in msdos gpt; do

  # Extract flag names of type $table_type from the texinfo documentation.
  case $table_type in
      msdos) search_term=MS-DOS; pri_or_name=pri;;
      gpt)   search_term=GPT;    pri_or_name=P1;;
  esac
  flags=$(sed -n '/^@node set/,/^@node/p' \
                    "$abs_top_srcdir/doc/parted.texi" \
                | perl -00 -ne \
                    '/^\@item (\w+).*'"$search_term"'/s and print lc($1), "\n"')

  n_sectors=5000
  dd if=/dev/null of=$dev bs=$ss seek=$n_sectors || fail=1

  parted -s $dev mklabel $table_type \
    mkpart $pri_or_name ext2 $((1*2048))s $((2*2048-1))s \
      > out 2> err || fail=1
  compare /dev/null out || fail=1

  for mode in on_only on_and_off ; do
    for flag in $flags; do

      # Exclude the supplemental flags.
      # These are not boolean, like the others.
      case $flag in boot|lba|hidden) continue;; esac

      # Turn on each flag, one at a time.
      parted -m -s $dev set 1 $flag on u s print > raw 2> err || fail=1
      extract_flags raw > out
      grep -F "$flag" out \
        || { warn_ "$ME: flag not turned on: $(cat out)"; fail=1; }
      compare /dev/null err || fail=1

      if test $mode = on_and_off; then
        # Turn it off
        parted -m -s $dev set 1 $flag off u s print > raw 2> err || fail=1
        extract_flags raw > out
        grep -F "$flag" out \
          && { warn_ "$ME: flag not turned off: $(cat out)"; fail=1; }
        compare /dev/null err || fail=1
      fi
    done
  done
done

Exit $fail
