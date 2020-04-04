#  Software floating-point emulation.
#  Makefile for SPARC v9 ABI mandated long double utility
#  functions (_Qp_*).
#  Copyright (C) 1999-2017 Free Software Foundation, Inc.
#  This file is part of the GNU C Library.
#  Contributed by Jakub Jelinek (jj@ultra.linux.cz).
#

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

ifeq ($(subdir),soft-fp)
sparc64-quad-routines := qp_add qp_cmp qp_cmpe qp_div qp_dtoq qp_feq qp_fge \
	qp_fgt qp_fle qp_flt qp_fne qp_itoq qp_mul qp_neg qp_qtod qp_qtoi   \
	qp_qtos qp_qtoui qp_qtoux qp_qtox qp_sqrt qp_stoq qp_sub qp_uitoq   \
	qp_uxtoq qp_xtoq qp_util
sysdep_routines += $(sparc64-quad-routines)
endif

ifeq ($(subdir),math)
CPPFLAGS += -I../soft-fp/
endif
