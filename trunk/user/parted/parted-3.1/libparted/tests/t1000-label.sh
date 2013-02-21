#!/bin/sh
# run the label unit tests in a directory supporting O_DIRECT

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

# This wrapper around the ./label binary is used to find a directory
# in which one can open a file with the O_DIRECT flag.

. "${top_srcdir=../..}/tests/init.sh"; path_prepend_ .

label || fail=1

Exit $fail
