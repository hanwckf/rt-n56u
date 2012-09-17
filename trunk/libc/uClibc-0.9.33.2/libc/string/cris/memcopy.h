/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   Modified for use in uClibc (C) 2007 Axis Communications AB.
   Minimal modifications: include path name and #undef of WORD_COPY_FWD/BWD

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "../generic/memcopy.h"

/* We override the word-copying macros, partly because misalignment in one
   pointer isn't cause for a special function, partly because we want to
   get rid of all the static functions in generic/memcopy.c; these macros
   are only used in memmove.c since we have arch-specific mempcpy, memcpy and
   memset.  */

#undef OP_T_THRES
#define OP_T_THRES OPSIZ

#undef WORD_COPY_FWD
#define WORD_COPY_FWD(dst_bp, src_bp, nbytes_left, nbytes)		\
  do									\
    {									\
      unsigned long enddst_bp = dst_bp + nbytes - (nbytes % OPSIZ);	\
      nbytes_left = (nbytes % OPSIZ);					\
      while (dst_bp < (unsigned long) enddst_bp)			\
	{								\
	  op_t x = *(op_t *) src_bp;					\
	  src_bp += sizeof x;						\
	  *(op_t *) dst_bp = x;						\
	  dst_bp += sizeof x;						\
	}								\
    } while (0)

#undef WORD_COPY_BWD
#define WORD_COPY_BWD(dst_bp, src_bp, nbytes_left, nbytes)		\
  do									\
    {									\
      unsigned long enddst_bp = dst_bp - nbytes + (nbytes % OPSIZ);	\
      nbytes_left = (nbytes % OPSIZ);					\
      while (dst_bp > enddst_bp)					\
	{								\
	  op_t x;							\
	  src_bp -= sizeof x;						\
	  x = *(op_t *) src_bp;						\
	  dst_bp -= sizeof x;						\
	  *(op_t *) dst_bp = x;						\
	}								\
    } while (0)
