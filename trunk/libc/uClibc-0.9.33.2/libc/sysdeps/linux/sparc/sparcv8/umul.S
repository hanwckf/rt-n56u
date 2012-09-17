/*
 * Sparc v8 has multiply.
 */

ENTRY(.umul)

	umul	%o0, %o1, %o0
	retl
	 rd	%y, %o1

END(.umul)
