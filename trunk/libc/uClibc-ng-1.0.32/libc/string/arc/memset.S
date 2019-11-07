/*
 * Copyright (C) 2013, 2014-2015, 2017 Synopsys, Inc. (www.synopsys.com)
 * Copyright (C) 2007 ARC International (UK) LTD
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

#if !defined(__ARC700__) && !defined(__ARCHS__)
#error "Neither ARC700 nor ARCHS is defined!"
#endif

ENTRY(memset)

#ifdef __ARC700__
#define SMALL	7 /* Must be at least 6 to deal with alignment/loop issues.  */

	mov_s	r4,r0
	or	r12,r0,r2
	bmsk.f	r12,r12,1
	extb_s	r1,r1
	asl	r3,r1,8
	beq.d	.Laligned
	or_s	r1,r1,r3
	brls	r2,SMALL,.Ltiny
	add	r3,r2,r0
	stb	r1,[r3,-1]
	bclr_s	r3,r3,0
	stw	r1,[r3,-2]
	bmsk.f	r12,r0,1
	add_s	r2,r2,r12
	sub.ne	r2,r2,4
	stb.ab	r1,[r4,1]
	and	r4,r4,-2
	stw.ab	r1,[r4,2]
	and	r4,r4,-4
.Laligned:	; This code address should be aligned for speed.
	asl	r3,r1,16
	lsr.f	lp_count,r2,2
	or_s	r1,r1,r3
	lpne	.Loop_end
	st.ab	r1,[r4,4]
.Loop_end:
	j_s	[blink]


	.balign	4
.Ltiny:
	mov.f	lp_count,r2
	lpne	.Ltiny_end
	stb.ab	r1,[r4,1]
.Ltiny_end:
	j_s	[blink]
#endif /* __ARC700__ */

#ifdef __ARCHS__
#ifdef DONT_USE_PREALLOC
#define PREWRITE(A,B)	prefetchw [(A),(B)]
#else
#define PREWRITE(A,B)	prealloc [(A),(B)]
#endif

	prefetchw [r0]		; Prefetch the write location
	mov.f	0, r2
;;; if size is zero
	jz.d	[blink]
	mov	r3, r0		; don't clobber ret val

;;; if length < 8
	brls.d.nt	r2, 8, .Lsmallchunk
	mov.f	lp_count,r2

	and.f	r4, r0, 0x03
	rsub	lp_count, r4, 4
	lpnz	@.Laligndestination
	;; LOOP BEGIN
	stb.ab	r1, [r3,1]
	sub	r2, r2, 1
.Laligndestination:

;;; Destination is aligned
	and	r1, r1, 0xFF
	asl	r4, r1, 8
	or	r4, r4, r1
	asl	r5, r4, 16
	or	r5, r5, r4
	mov	r4, r5

	sub3	lp_count, r2, 8
	cmp     r2, 64
	bmsk.hi	r2, r2, 5
	mov.ls	lp_count, 0
	add3.hi	r2, r2, 8

;;; Convert len to Dwords, unfold x8
	lsr.f	lp_count, lp_count, 6
	lpnz	@.Lset64bytes
	;; LOOP START
	PREWRITE(r3, 64)	;Prefetch the next write location
#if defined(__LL64__) || defined(__ARC_LL64__)
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
#else
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
#endif
.Lset64bytes:

	lsr.f	lp_count, r2, 5 ;Last remaining  max 124 bytes
	lpnz	.Lset32bytes
	;; LOOP START
	prefetchw [r3, 32]	;Prefetch the next write location
#if defined(__LL64__) || defined(__ARC_LL64__)
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
	std.ab	r4, [r3, 8]
#else
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
	st.ab	r4, [r3, 4]
#endif
.Lset32bytes:

	and.f	lp_count, r2, 0x1F ;Last remaining 31 bytes
.Lsmallchunk:
	lpnz	.Lcopy3bytes
	;; LOOP START
	stb.ab	r1, [r3, 1]
.Lcopy3bytes:

	j	[blink]
#endif /* __ARCHS__ */

END(memset)
libc_hidden_def(memset)
