/*
 * Sparc v8 has divide.
 */

ENTRY(.urem)

	wr	%g0, 0, %y
	nop
	nop
	nop
	udiv	%o0, %o1, %o2
	umul	%o2, %o1, %o2
	retl
	 sub	%o0, %o2, %o0

END(.urem)
