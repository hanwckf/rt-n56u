/*
 * Sparc v8 has divide.
 */

ENTRY(.udiv)

	wr	%g0, 0, %y
	nop
	nop
	retl
	 udiv	%o0, %o1, %o0

END(.udiv)
