# Copyright (C) 2003 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, see <http://www.gnu.org/licenses/>.

ifeq ($(subdir),nptl)
# It turns out that stack coloring is in general not good on P4s.  Some
# applications will benefit.  We will probably have a configuration option
# at some point.  Enabling coloring can be done with
#
#   -DCOLORING_INCREMENT=128
#
# What is useful is to avoid the 64k aliasing problem which reliably
# happens if all stacks use sizes which are a multiple of 64k.  Tell
# the stack allocator to disturb this by allocation one more page if
# necessary.
CFLAGS-pthread_create.c += -DMULTI_PAGE_ALIASING=65536
endif
