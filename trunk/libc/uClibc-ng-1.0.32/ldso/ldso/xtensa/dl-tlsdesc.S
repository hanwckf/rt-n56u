/* Thread-local storage handling in the ELF dynamic linker.  Xtensa version.
   Copyright (C) 2012-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#if defined __UCLIBC_HAS_TLS__
#include <tls.h>
#include "tlsdesc.h"

	.text
HIDDEN_ENTRY (_dl_tlsdesc_return)
	rur.threadptr	a3
	add		a2, a2, a3
	abi_ret
END (_dl_tlsdesc_return)

#ifdef SHARED


	/* This function is used for symbols that need dynamic TLS.

	   The argument passed to this function points to the TLS descriptor.

	   The assembly code that follows is a rendition of the following
	   C code, hand-optimized a little bit.

	   void *
	   _dl_tlsdesc_dynamic(struct tlsdesc_dynamic_arg *td)
	   {
	     dtv_t *dtv = (dtv_t *)THREAD_DTV();
	     if (td->gen_count <= dtv[0].counter
	         && dtv[td->tlsinfo.ti_module].pointer.val
	            != TLS_DTV_UNALLOCATED)
	       return dtv[td->tlsinfo.ti_module].pointer.val
	              + td->tlsinfo.ti_offset;
	     return __tls_get_addr (&td->tlsinfo);
	   }
	 */

HIDDEN_ENTRY (_dl_tlsdesc_dynamic)

	/* dtv_t *dtv = (dtv_t *)THREAD_DTV(); */
	rur.threadptr	a3
	l32i	a4, a3, 0

	/* if (td->gen_count <= dtv[0].counter */
	l32i	a6, a2, TLSDESC_GEN_COUNT
	l32i	a7, a4, 0
	blt	a7, a6, .Lslow

	/* && dtv[td->tlsinfo.ti_module].pointer.val != TLS_DTV_UNALLOCATED) */
	l32i	a6, a2, TLSDESC_MODID
	addx8	a6, a6, a4
	l32i	a6, a6, 0
	beqi	a6, -1, .Lslow

	/* return dtv[td->tlsinfo.ti_module].pointer.val
	     + td->tlsinfo.ti_offset; */
	l32i	a5, a2, TLSDESC_MODOFF
	add	a2, a6, a5
	abi_ret

	/* return __tls_get_addr (&td->tlsinfo); */
.Lslow:
#if defined(__XTENSA_WINDOWED_ABI__)
	mov	a6, a2
	movi	a4, __tls_get_addr
	callx4	a4
	mov	a2, a6
	retw
#elif defined(__XTENSA_CALL0_ABI__)
	addi	a1, a1, -16
	s32i	a0, a1, 0
	movi	a0, __tls_get_addr
	callx0	a0
	l32i	a0, a1, 0
	addi	a1, a1, 16
	ret
#else
#error Unsupported Xtensa ABI
#endif
END (_dl_tlsdesc_dynamic)

#endif /* SHARED */
#endif
