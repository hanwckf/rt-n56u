	.align 4
__syscall_error:
	/* Call errno_location, store '-r4' in errno and return -1 */
	mov.l	r12, @-r15
	sts.l	pr, @-r15
#ifdef SHARED
	mova	.LG, r0
	mov.l	.LG, r12
	add	r0, r12
	mov.l	1f, r0
	mov.l	@(r0,r12),r0
 	jsr	@r0
	 neg	r4, r12
#else	
	mov.l	1f, r0
	bsrf	r0
	 neg	r4, r12
.jmp_loc:		
#endif
	mov.l	r12, @r0
	lds.l	@r15+, pr
	mov.l	@r15+,r12

	/* And just kick back a -1.  */
	rts
	 mov	#-1, r0

	.align	4

#ifdef SHARED
1:	.long   __errno_location@GOT
.LG:	.long	_GLOBAL_OFFSET_TABLE_
#else
1:	.long   __errno_location - .jmp_loc
#endif		


