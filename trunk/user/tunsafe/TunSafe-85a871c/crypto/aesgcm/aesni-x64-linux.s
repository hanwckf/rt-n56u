.text	
.globl	aesni_encrypt
.type	aesni_encrypt,@function
.align	16
aesni_encrypt:
	movups	(%rdi),%xmm2
	movl	240(%rdx),%eax
	movups	(%rdx),%xmm0
	movups	16(%rdx),%xmm1
	leaq	32(%rdx),%rdx
	xorps	%xmm0,%xmm2
.Loop_enc1_1:
.byte	102,15,56,220,209
	decl	%eax
	movups	(%rdx),%xmm1
	leaq	16(%rdx),%rdx
	jnz	.Loop_enc1_1
.byte	102,15,56,221,209
	pxor	%xmm0,%xmm0
	pxor	%xmm1,%xmm1
	movups	%xmm2,(%rsi)
	pxor	%xmm2,%xmm2
	ret
.size	aesni_encrypt,.-aesni_encrypt

.globl	aesni_decrypt
.type	aesni_decrypt,@function
.align	16
aesni_decrypt:
	movups	(%rdi),%xmm2
	movl	240(%rdx),%eax
	movups	(%rdx),%xmm0
	movups	16(%rdx),%xmm1
	leaq	32(%rdx),%rdx
	xorps	%xmm0,%xmm2
.Loop_dec1_2:
.byte	102,15,56,222,209
	decl	%eax
	movups	(%rdx),%xmm1
	leaq	16(%rdx),%rdx
	jnz	.Loop_dec1_2
.byte	102,15,56,223,209
	pxor	%xmm0,%xmm0
	pxor	%xmm1,%xmm1
	movups	%xmm2,(%rsi)
	pxor	%xmm2,%xmm2
	ret
.size	aesni_decrypt, .-aesni_decrypt
.type	_aesni_encrypt2,@function
.align	16
_aesni_encrypt2:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	movups	32(%rcx),%xmm0
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
	addq	$16,%rax

.Lenc_loop2:
.byte	102,15,56,220,209
.byte	102,15,56,220,217
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,220,208
.byte	102,15,56,220,216
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Lenc_loop2

.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,221,208
.byte	102,15,56,221,216
	ret
.size	_aesni_encrypt2,.-_aesni_encrypt2
.type	_aesni_decrypt2,@function
.align	16
_aesni_decrypt2:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	movups	32(%rcx),%xmm0
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
	addq	$16,%rax

.Ldec_loop2:
.byte	102,15,56,222,209
.byte	102,15,56,222,217
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,222,208
.byte	102,15,56,222,216
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Ldec_loop2

.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,223,208
.byte	102,15,56,223,216
	ret
.size	_aesni_decrypt2,.-_aesni_decrypt2
.type	_aesni_encrypt3,@function
.align	16
_aesni_encrypt3:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	xorps	%xmm0,%xmm4
	movups	32(%rcx),%xmm0
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
	addq	$16,%rax

.Lenc_loop3:
.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,220,208
.byte	102,15,56,220,216
.byte	102,15,56,220,224
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Lenc_loop3

.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,221,208
.byte	102,15,56,221,216
.byte	102,15,56,221,224
	ret
.size	_aesni_encrypt3,.-_aesni_encrypt3
.type	_aesni_decrypt3,@function
.align	16
_aesni_decrypt3:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	xorps	%xmm0,%xmm4
	movups	32(%rcx),%xmm0
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
	addq	$16,%rax

.Ldec_loop3:
.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,222,225
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,222,208
.byte	102,15,56,222,216
.byte	102,15,56,222,224
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Ldec_loop3

.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,222,225
.byte	102,15,56,223,208
.byte	102,15,56,223,216
.byte	102,15,56,223,224
	ret
.size	_aesni_decrypt3,.-_aesni_decrypt3
.type	_aesni_encrypt4,@function
.align	16
_aesni_encrypt4:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	xorps	%xmm0,%xmm4
	xorps	%xmm0,%xmm5
	movups	32(%rcx),%xmm0
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
.byte	0x0f,0x1f,0x00
	addq	$16,%rax

.Lenc_loop4:
.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,220,208
.byte	102,15,56,220,216
.byte	102,15,56,220,224
.byte	102,15,56,220,232
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Lenc_loop4

.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
.byte	102,15,56,221,208
.byte	102,15,56,221,216
.byte	102,15,56,221,224
.byte	102,15,56,221,232
	ret
.size	_aesni_encrypt4,.-_aesni_encrypt4
.type	_aesni_decrypt4,@function
.align	16
_aesni_decrypt4:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	xorps	%xmm0,%xmm4
	xorps	%xmm0,%xmm5
	movups	32(%rcx),%xmm0
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
.byte	0x0f,0x1f,0x00
	addq	$16,%rax

.Ldec_loop4:
.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,222,225
.byte	102,15,56,222,233
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,222,208
.byte	102,15,56,222,216
.byte	102,15,56,222,224
.byte	102,15,56,222,232
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Ldec_loop4

.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,222,225
.byte	102,15,56,222,233
.byte	102,15,56,223,208
.byte	102,15,56,223,216
.byte	102,15,56,223,224
.byte	102,15,56,223,232
	ret
.size	_aesni_decrypt4,.-_aesni_decrypt4
.type	_aesni_encrypt6,@function
.align	16
_aesni_encrypt6:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	pxor	%xmm0,%xmm3
	pxor	%xmm0,%xmm4
.byte	102,15,56,220,209
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
.byte	102,15,56,220,217
	pxor	%xmm0,%xmm5
	pxor	%xmm0,%xmm6
.byte	102,15,56,220,225
	pxor	%xmm0,%xmm7
	movups	(%rcx,%rax,1),%xmm0
	addq	$16,%rax
	jmp	.Lenc_loop6_enter
.align	16
.Lenc_loop6:
.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.Lenc_loop6_enter:
.byte	102,15,56,220,233
.byte	102,15,56,220,241
.byte	102,15,56,220,249
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,220,208
.byte	102,15,56,220,216
.byte	102,15,56,220,224
.byte	102,15,56,220,232
.byte	102,15,56,220,240
.byte	102,15,56,220,248
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Lenc_loop6

.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,15,56,221,208
.byte	102,15,56,221,216
.byte	102,15,56,221,224
.byte	102,15,56,221,232
.byte	102,15,56,221,240
.byte	102,15,56,221,248
	ret
.size	_aesni_encrypt6,.-_aesni_encrypt6
.type	_aesni_decrypt6,@function
.align	16
_aesni_decrypt6:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	pxor	%xmm0,%xmm3
	pxor	%xmm0,%xmm4
.byte	102,15,56,222,209
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
.byte	102,15,56,222,217
	pxor	%xmm0,%xmm5
	pxor	%xmm0,%xmm6
.byte	102,15,56,222,225
	pxor	%xmm0,%xmm7
	movups	(%rcx,%rax,1),%xmm0
	addq	$16,%rax
	jmp	.Ldec_loop6_enter
.align	16
.Ldec_loop6:
.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,222,225
.Ldec_loop6_enter:
.byte	102,15,56,222,233
.byte	102,15,56,222,241
.byte	102,15,56,222,249
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,222,208
.byte	102,15,56,222,216
.byte	102,15,56,222,224
.byte	102,15,56,222,232
.byte	102,15,56,222,240
.byte	102,15,56,222,248
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Ldec_loop6

.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,222,225
.byte	102,15,56,222,233
.byte	102,15,56,222,241
.byte	102,15,56,222,249
.byte	102,15,56,223,208
.byte	102,15,56,223,216
.byte	102,15,56,223,224
.byte	102,15,56,223,232
.byte	102,15,56,223,240
.byte	102,15,56,223,248
	ret
.size	_aesni_decrypt6,.-_aesni_decrypt6
.type	_aesni_encrypt8,@function
.align	16
_aesni_encrypt8:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	pxor	%xmm0,%xmm4
	pxor	%xmm0,%xmm5
	pxor	%xmm0,%xmm6
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
.byte	102,15,56,220,209
	pxor	%xmm0,%xmm7
	pxor	%xmm0,%xmm8
.byte	102,15,56,220,217
	pxor	%xmm0,%xmm9
	movups	(%rcx,%rax,1),%xmm0
	addq	$16,%rax
	jmp	.Lenc_loop8_inner
.align	16
.Lenc_loop8:
.byte	102,15,56,220,209
.byte	102,15,56,220,217
.Lenc_loop8_inner:
.byte	102,15,56,220,225
.byte	102,15,56,220,233
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
.Lenc_loop8_enter:
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,220,208
.byte	102,15,56,220,216
.byte	102,15,56,220,224
.byte	102,15,56,220,232
.byte	102,15,56,220,240
.byte	102,15,56,220,248
.byte	102,68,15,56,220,192
.byte	102,68,15,56,220,200
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Lenc_loop8

.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
.byte	102,15,56,221,208
.byte	102,15,56,221,216
.byte	102,15,56,221,224
.byte	102,15,56,221,232
.byte	102,15,56,221,240
.byte	102,15,56,221,248
.byte	102,68,15,56,221,192
.byte	102,68,15,56,221,200
	ret
.size	_aesni_encrypt8,.-_aesni_encrypt8
.type	_aesni_decrypt8,@function
.align	16
_aesni_decrypt8:
	movups	(%rcx),%xmm0
	shll	$4,%eax
	movups	16(%rcx),%xmm1
	xorps	%xmm0,%xmm2
	xorps	%xmm0,%xmm3
	pxor	%xmm0,%xmm4
	pxor	%xmm0,%xmm5
	pxor	%xmm0,%xmm6
	leaq	32(%rcx,%rax,1),%rcx
	negq	%rax
.byte	102,15,56,222,209
	pxor	%xmm0,%xmm7
	pxor	%xmm0,%xmm8
.byte	102,15,56,222,217
	pxor	%xmm0,%xmm9
	movups	(%rcx,%rax,1),%xmm0
	addq	$16,%rax
	jmp	.Ldec_loop8_inner
.align	16
.Ldec_loop8:
.byte	102,15,56,222,209
.byte	102,15,56,222,217
.Ldec_loop8_inner:
.byte	102,15,56,222,225
.byte	102,15,56,222,233
.byte	102,15,56,222,241
.byte	102,15,56,222,249
.byte	102,68,15,56,222,193
.byte	102,68,15,56,222,201
.Ldec_loop8_enter:
	movups	(%rcx,%rax,1),%xmm1
	addq	$32,%rax
.byte	102,15,56,222,208
.byte	102,15,56,222,216
.byte	102,15,56,222,224
.byte	102,15,56,222,232
.byte	102,15,56,222,240
.byte	102,15,56,222,248
.byte	102,68,15,56,222,192
.byte	102,68,15,56,222,200
	movups	-16(%rcx,%rax,1),%xmm0
	jnz	.Ldec_loop8

.byte	102,15,56,222,209
.byte	102,15,56,222,217
.byte	102,15,56,222,225
.byte	102,15,56,222,233
.byte	102,15,56,222,241
.byte	102,15,56,222,249
.byte	102,68,15,56,222,193
.byte	102,68,15,56,222,201
.byte	102,15,56,223,208
.byte	102,15,56,223,216
.byte	102,15,56,223,224
.byte	102,15,56,223,232
.byte	102,15,56,223,240
.byte	102,15,56,223,248
.byte	102,68,15,56,223,192
.byte	102,68,15,56,223,200
	ret
.size	_aesni_decrypt8,.-_aesni_decrypt8
.globl	aesni_ctr32_encrypt_blocks
.type	aesni_ctr32_encrypt_blocks,@function
.align	16
aesni_ctr32_encrypt_blocks:
.cfi_startproc	
	cmpq	$1,%rdx
	jne	.Lctr32_bulk



	movups	(%r8),%xmm2
	movups	(%rdi),%xmm3
	movl	240(%rcx),%edx
	movups	(%rcx),%xmm0
	movups	16(%rcx),%xmm1
	leaq	32(%rcx),%rcx
	xorps	%xmm0,%xmm2
.Loop_enc1_3:
.byte	102,15,56,220,209
	decl	%edx
	movups	(%rcx),%xmm1
	leaq	16(%rcx),%rcx
	jnz	.Loop_enc1_3
.byte	102,15,56,221,209
	pxor	%xmm0,%xmm0
	pxor	%xmm1,%xmm1
	xorps	%xmm3,%xmm2
	pxor	%xmm3,%xmm3
	movups	%xmm2,(%rsi)
	xorps	%xmm2,%xmm2
	jmp	.Lctr32_epilogue

.align	16
.Lctr32_bulk:
	leaq	(%rsp),%r11
.cfi_def_cfa_register	%r11
	pushq	%rbp
.cfi_offset	%rbp,-16
	subq	$128,%rsp
	andq	$-16,%rsp




	movdqu	(%r8),%xmm2
	movdqu	(%rcx),%xmm0
	movl	12(%r8),%r8d
	pxor	%xmm0,%xmm2
	movl	12(%rcx),%ebp
	movdqa	%xmm2,0(%rsp)
	bswapl	%r8d
	movdqa	%xmm2,%xmm3
	movdqa	%xmm2,%xmm4
	movdqa	%xmm2,%xmm5
	movdqa	%xmm2,64(%rsp)
	movdqa	%xmm2,80(%rsp)
	movdqa	%xmm2,96(%rsp)
	movq	%rdx,%r10
	movdqa	%xmm2,112(%rsp)

	leaq	1(%r8),%rax
	leaq	2(%r8),%rdx
	bswapl	%eax
	bswapl	%edx
	xorl	%ebp,%eax
	xorl	%ebp,%edx
.byte	102,15,58,34,216,3
	leaq	3(%r8),%rax
	movdqa	%xmm3,16(%rsp)
.byte	102,15,58,34,226,3
	bswapl	%eax
	movq	%r10,%rdx
	leaq	4(%r8),%r10
	movdqa	%xmm4,32(%rsp)
	xorl	%ebp,%eax
	bswapl	%r10d
.byte	102,15,58,34,232,3
	xorl	%ebp,%r10d
	movdqa	%xmm5,48(%rsp)
	leaq	5(%r8),%r9
	movl	%r10d,64+12(%rsp)
	bswapl	%r9d
	leaq	6(%r8),%r10
	movl	240(%rcx),%eax
	xorl	%ebp,%r9d
	bswapl	%r10d
	movl	%r9d,80+12(%rsp)
	xorl	%ebp,%r10d
	leaq	7(%r8),%r9
	movl	%r10d,96+12(%rsp)
	bswapl	%r9d


	xorl	%ebp,%r9d

	movl	%r9d,112+12(%rsp)

	movups	16(%rcx),%xmm1

	movdqa	64(%rsp),%xmm6
	movdqa	80(%rsp),%xmm7

	cmpq	$8,%rdx
	jb	.Lctr32_tail

	subq	$6,%rdx



	leaq	128(%rcx),%rcx
	subq	$2,%rdx
	jmp	.Lctr32_loop8










.align	16
.Lctr32_loop6:
	addl	$6,%r8d
	movups	-48(%rcx,%r10,1),%xmm0
.byte	102,15,56,220,209
	movl	%r8d,%eax
	xorl	%ebp,%eax
.byte	102,15,56,220,217
.byte	0x0f,0x38,0xf1,0x44,0x24,12
	leal	1(%r8),%eax
.byte	102,15,56,220,225
	xorl	%ebp,%eax
.byte	0x0f,0x38,0xf1,0x44,0x24,28
.byte	102,15,56,220,233
	leal	2(%r8),%eax
	xorl	%ebp,%eax
.byte	102,15,56,220,241
.byte	0x0f,0x38,0xf1,0x44,0x24,44
	leal	3(%r8),%eax
.byte	102,15,56,220,249
	movups	-32(%rcx,%r10,1),%xmm1
	xorl	%ebp,%eax

.byte	102,15,56,220,208
.byte	0x0f,0x38,0xf1,0x44,0x24,60
	leal	4(%r8),%eax
.byte	102,15,56,220,216
	xorl	%ebp,%eax
.byte	0x0f,0x38,0xf1,0x44,0x24,76
.byte	102,15,56,220,224
	leal	5(%r8),%eax
	xorl	%ebp,%eax
.byte	102,15,56,220,232
.byte	0x0f,0x38,0xf1,0x44,0x24,92
	movq	%r10,%rax
.byte	102,15,56,220,240
.byte	102,15,56,220,248
	movups	-16(%rcx,%r10,1),%xmm0

	call	.Lenc_loop6

	movdqu	(%rdi),%xmm8
	movdqu	16(%rdi),%xmm9
	movdqu	32(%rdi),%xmm10
	movdqu	48(%rdi),%xmm11
	movdqu	64(%rdi),%xmm12
	movdqu	80(%rdi),%xmm13
	leaq	96(%rdi),%rdi
	movups	-64(%rcx,%r10,1),%xmm1
	pxor	%xmm2,%xmm8
	movaps	0(%rsp),%xmm2
	pxor	%xmm3,%xmm9
	movaps	16(%rsp),%xmm3
	pxor	%xmm4,%xmm10
	movaps	32(%rsp),%xmm4
	pxor	%xmm5,%xmm11
	movaps	48(%rsp),%xmm5
	pxor	%xmm6,%xmm12
	movaps	64(%rsp),%xmm6
	pxor	%xmm7,%xmm13
	movaps	80(%rsp),%xmm7
	movdqu	%xmm8,(%rsi)
	movdqu	%xmm9,16(%rsi)
	movdqu	%xmm10,32(%rsi)
	movdqu	%xmm11,48(%rsi)
	movdqu	%xmm12,64(%rsi)
	movdqu	%xmm13,80(%rsi)
	leaq	96(%rsi),%rsi

	subq	$6,%rdx
	jnc	.Lctr32_loop6

	addq	$6,%rdx
	jz	.Lctr32_done

	leal	-48(%r10),%eax
	leaq	-80(%rcx,%r10,1),%rcx
	negl	%eax
	shrl	$4,%eax
	jmp	.Lctr32_tail

.align	32
.Lctr32_loop8:
	addl	$8,%r8d
	movdqa	96(%rsp),%xmm8
.byte	102,15,56,220,209
	movl	%r8d,%r9d
	movdqa	112(%rsp),%xmm9
.byte	102,15,56,220,217
	bswapl	%r9d
	movups	32-128(%rcx),%xmm0
.byte	102,15,56,220,225
	xorl	%ebp,%r9d
	nop
.byte	102,15,56,220,233
	movl	%r9d,0+12(%rsp)
	leaq	1(%r8),%r9
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
	movups	48-128(%rcx),%xmm1
	bswapl	%r9d
.byte	102,15,56,220,208
.byte	102,15,56,220,216
	xorl	%ebp,%r9d
.byte	0x66,0x90
.byte	102,15,56,220,224
.byte	102,15,56,220,232
	movl	%r9d,16+12(%rsp)
	leaq	2(%r8),%r9
.byte	102,15,56,220,240
.byte	102,15,56,220,248
.byte	102,68,15,56,220,192
.byte	102,68,15,56,220,200
	movups	64-128(%rcx),%xmm0
	bswapl	%r9d
.byte	102,15,56,220,209
.byte	102,15,56,220,217
	xorl	%ebp,%r9d
.byte	0x66,0x90
.byte	102,15,56,220,225
.byte	102,15,56,220,233
	movl	%r9d,32+12(%rsp)
	leaq	3(%r8),%r9
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
	movups	80-128(%rcx),%xmm1
	bswapl	%r9d
.byte	102,15,56,220,208
.byte	102,15,56,220,216
	xorl	%ebp,%r9d
.byte	0x66,0x90
.byte	102,15,56,220,224
.byte	102,15,56,220,232
	movl	%r9d,48+12(%rsp)
	leaq	4(%r8),%r9
.byte	102,15,56,220,240
.byte	102,15,56,220,248
.byte	102,68,15,56,220,192
.byte	102,68,15,56,220,200
	movups	96-128(%rcx),%xmm0
	bswapl	%r9d
.byte	102,15,56,220,209
.byte	102,15,56,220,217
	xorl	%ebp,%r9d
.byte	0x66,0x90
.byte	102,15,56,220,225
.byte	102,15,56,220,233
	movl	%r9d,64+12(%rsp)
	leaq	5(%r8),%r9
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
	movups	112-128(%rcx),%xmm1
	bswapl	%r9d
.byte	102,15,56,220,208
.byte	102,15,56,220,216
	xorl	%ebp,%r9d
.byte	0x66,0x90
.byte	102,15,56,220,224
.byte	102,15,56,220,232
	movl	%r9d,80+12(%rsp)
	leaq	6(%r8),%r9
.byte	102,15,56,220,240
.byte	102,15,56,220,248
.byte	102,68,15,56,220,192
.byte	102,68,15,56,220,200
	movups	128-128(%rcx),%xmm0
	bswapl	%r9d
.byte	102,15,56,220,209
.byte	102,15,56,220,217
	xorl	%ebp,%r9d
.byte	0x66,0x90
.byte	102,15,56,220,225
.byte	102,15,56,220,233
	movl	%r9d,96+12(%rsp)
	leaq	7(%r8),%r9
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
	movups	144-128(%rcx),%xmm1
	bswapl	%r9d
.byte	102,15,56,220,208
.byte	102,15,56,220,216
.byte	102,15,56,220,224
	xorl	%ebp,%r9d
	movdqu	0(%rdi),%xmm10
.byte	102,15,56,220,232
	movl	%r9d,112+12(%rsp)
	cmpl	$11,%eax
.byte	102,15,56,220,240
.byte	102,15,56,220,248
.byte	102,68,15,56,220,192
.byte	102,68,15,56,220,200
	movups	160-128(%rcx),%xmm0

	jb	.Lctr32_enc_done

.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
	movups	176-128(%rcx),%xmm1

.byte	102,15,56,220,208
.byte	102,15,56,220,216
.byte	102,15,56,220,224
.byte	102,15,56,220,232
.byte	102,15,56,220,240
.byte	102,15,56,220,248
.byte	102,68,15,56,220,192
.byte	102,68,15,56,220,200
	movups	192-128(%rcx),%xmm0
	je	.Lctr32_enc_done

.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
	movups	208-128(%rcx),%xmm1

.byte	102,15,56,220,208
.byte	102,15,56,220,216
.byte	102,15,56,220,224
.byte	102,15,56,220,232
.byte	102,15,56,220,240
.byte	102,15,56,220,248
.byte	102,68,15,56,220,192
.byte	102,68,15,56,220,200
	movups	224-128(%rcx),%xmm0
	jmp	.Lctr32_enc_done

.align	16
.Lctr32_enc_done:
	movdqu	16(%rdi),%xmm11
	pxor	%xmm0,%xmm10
	movdqu	32(%rdi),%xmm12
	pxor	%xmm0,%xmm11
	movdqu	48(%rdi),%xmm13
	pxor	%xmm0,%xmm12
	movdqu	64(%rdi),%xmm14
	pxor	%xmm0,%xmm13
	movdqu	80(%rdi),%xmm15
	pxor	%xmm0,%xmm14
	pxor	%xmm0,%xmm15
.byte	102,15,56,220,209
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
.byte	102,15,56,220,241
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193
.byte	102,68,15,56,220,201
	movdqu	96(%rdi),%xmm1
	leaq	128(%rdi),%rdi

.byte	102,65,15,56,221,210
	pxor	%xmm0,%xmm1
	movdqu	112-128(%rdi),%xmm10
.byte	102,65,15,56,221,219
	pxor	%xmm0,%xmm10
	movdqa	0(%rsp),%xmm11
.byte	102,65,15,56,221,228
.byte	102,65,15,56,221,237
	movdqa	16(%rsp),%xmm12
	movdqa	32(%rsp),%xmm13
.byte	102,65,15,56,221,246
.byte	102,65,15,56,221,255
	movdqa	48(%rsp),%xmm14
	movdqa	64(%rsp),%xmm15
.byte	102,68,15,56,221,193
	movdqa	80(%rsp),%xmm0
	movups	16-128(%rcx),%xmm1
.byte	102,69,15,56,221,202

	movups	%xmm2,(%rsi)
	movdqa	%xmm11,%xmm2
	movups	%xmm3,16(%rsi)
	movdqa	%xmm12,%xmm3
	movups	%xmm4,32(%rsi)
	movdqa	%xmm13,%xmm4
	movups	%xmm5,48(%rsi)
	movdqa	%xmm14,%xmm5
	movups	%xmm6,64(%rsi)
	movdqa	%xmm15,%xmm6
	movups	%xmm7,80(%rsi)
	movdqa	%xmm0,%xmm7
	movups	%xmm8,96(%rsi)
	movups	%xmm9,112(%rsi)
	leaq	128(%rsi),%rsi

	subq	$8,%rdx
	jnc	.Lctr32_loop8

	addq	$8,%rdx
	jz	.Lctr32_done
	leaq	-128(%rcx),%rcx

.Lctr32_tail:


	leaq	16(%rcx),%rcx
	cmpq	$4,%rdx
	jb	.Lctr32_loop3
	je	.Lctr32_loop4


	shll	$4,%eax
	movdqa	96(%rsp),%xmm8
	pxor	%xmm9,%xmm9

	movups	16(%rcx),%xmm0
.byte	102,15,56,220,209
.byte	102,15,56,220,217
	leaq	32-16(%rcx,%rax,1),%rcx
	negq	%rax
.byte	102,15,56,220,225
	addq	$16,%rax
	movups	(%rdi),%xmm10
.byte	102,15,56,220,233
.byte	102,15,56,220,241
	movups	16(%rdi),%xmm11
	movups	32(%rdi),%xmm12
.byte	102,15,56,220,249
.byte	102,68,15,56,220,193

	call	.Lenc_loop8_enter

	movdqu	48(%rdi),%xmm13
	pxor	%xmm10,%xmm2
	movdqu	64(%rdi),%xmm10
	pxor	%xmm11,%xmm3
	movdqu	%xmm2,(%rsi)
	pxor	%xmm12,%xmm4
	movdqu	%xmm3,16(%rsi)
	pxor	%xmm13,%xmm5
	movdqu	%xmm4,32(%rsi)
	pxor	%xmm10,%xmm6
	movdqu	%xmm5,48(%rsi)
	movdqu	%xmm6,64(%rsi)
	cmpq	$6,%rdx
	jb	.Lctr32_done

	movups	80(%rdi),%xmm11
	xorps	%xmm11,%xmm7
	movups	%xmm7,80(%rsi)
	je	.Lctr32_done

	movups	96(%rdi),%xmm12
	xorps	%xmm12,%xmm8
	movups	%xmm8,96(%rsi)
	jmp	.Lctr32_done

.align	32
.Lctr32_loop4:
.byte	102,15,56,220,209
	leaq	16(%rcx),%rcx
	decl	%eax
.byte	102,15,56,220,217
.byte	102,15,56,220,225
.byte	102,15,56,220,233
	movups	(%rcx),%xmm1
	jnz	.Lctr32_loop4
.byte	102,15,56,221,209
.byte	102,15,56,221,217
	movups	(%rdi),%xmm10
	movups	16(%rdi),%xmm11
.byte	102,15,56,221,225
.byte	102,15,56,221,233
	movups	32(%rdi),%xmm12
	movups	48(%rdi),%xmm13

	xorps	%xmm10,%xmm2
	movups	%xmm2,(%rsi)
	xorps	%xmm11,%xmm3
	movups	%xmm3,16(%rsi)
	pxor	%xmm12,%xmm4
	movdqu	%xmm4,32(%rsi)
	pxor	%xmm13,%xmm5
	movdqu	%xmm5,48(%rsi)
	jmp	.Lctr32_done

.align	32
.Lctr32_loop3:
.byte	102,15,56,220,209
	leaq	16(%rcx),%rcx
	decl	%eax
.byte	102,15,56,220,217
.byte	102,15,56,220,225
	movups	(%rcx),%xmm1
	jnz	.Lctr32_loop3
.byte	102,15,56,221,209
.byte	102,15,56,221,217
.byte	102,15,56,221,225

	movups	(%rdi),%xmm10
	xorps	%xmm10,%xmm2
	movups	%xmm2,(%rsi)
	cmpq	$2,%rdx
	jb	.Lctr32_done

	movups	16(%rdi),%xmm11
	xorps	%xmm11,%xmm3
	movups	%xmm3,16(%rsi)
	je	.Lctr32_done

	movups	32(%rdi),%xmm12
	xorps	%xmm12,%xmm4
	movups	%xmm4,32(%rsi)

.Lctr32_done:
	xorps	%xmm0,%xmm0
	xorl	%ebp,%ebp
	pxor	%xmm1,%xmm1
	pxor	%xmm2,%xmm2
	pxor	%xmm3,%xmm3
	pxor	%xmm4,%xmm4
	pxor	%xmm5,%xmm5
	pxor	%xmm6,%xmm6
	pxor	%xmm7,%xmm7
	movaps	%xmm0,0(%rsp)
	pxor	%xmm8,%xmm8
	movaps	%xmm0,16(%rsp)
	pxor	%xmm9,%xmm9
	movaps	%xmm0,32(%rsp)
	pxor	%xmm10,%xmm10
	movaps	%xmm0,48(%rsp)
	pxor	%xmm11,%xmm11
	movaps	%xmm0,64(%rsp)
	pxor	%xmm12,%xmm12
	movaps	%xmm0,80(%rsp)
	pxor	%xmm13,%xmm13
	movaps	%xmm0,96(%rsp)
	pxor	%xmm14,%xmm14
	movaps	%xmm0,112(%rsp)
	pxor	%xmm15,%xmm15
	movq	-8(%r11),%rbp
.cfi_restore	%rbp
	leaq	(%r11),%rsp
.cfi_def_cfa_register	%rsp
.Lctr32_epilogue:
	ret
.cfi_endproc	
.size	aesni_ctr32_encrypt_blocks,.-aesni_ctr32_encrypt_blocks
.globl	aesni_set_decrypt_key
.type	aesni_set_decrypt_key,@function
.align	16
aesni_set_decrypt_key:
.cfi_startproc	
.byte	0x48,0x83,0xEC,0x08
.cfi_adjust_cfa_offset	8
	call	__aesni_set_encrypt_key
	shll	$4,%esi
	testl	%eax,%eax
	jnz	.Ldec_key_ret
	leaq	16(%rdx,%rsi,1),%rdi

	movups	(%rdx),%xmm0
	movups	(%rdi),%xmm1
	movups	%xmm0,(%rdi)
	movups	%xmm1,(%rdx)
	leaq	16(%rdx),%rdx
	leaq	-16(%rdi),%rdi

.Ldec_key_inverse:
	movups	(%rdx),%xmm0
	movups	(%rdi),%xmm1
.byte	102,15,56,219,192
.byte	102,15,56,219,201
	leaq	16(%rdx),%rdx
	leaq	-16(%rdi),%rdi
	movups	%xmm0,16(%rdi)
	movups	%xmm1,-16(%rdx)
	cmpq	%rdx,%rdi
	ja	.Ldec_key_inverse

	movups	(%rdx),%xmm0
.byte	102,15,56,219,192
	pxor	%xmm1,%xmm1
	movups	%xmm0,(%rdi)
	pxor	%xmm0,%xmm0
.Ldec_key_ret:
	addq	$8,%rsp
.cfi_adjust_cfa_offset	-8
	ret
.cfi_endproc	
.LSEH_end_set_decrypt_key:
.size	aesni_set_decrypt_key,.-aesni_set_decrypt_key
.globl	aesni_set_encrypt_key
.type	aesni_set_encrypt_key,@function
.align	16
aesni_set_encrypt_key:
__aesni_set_encrypt_key:
.cfi_startproc	
.byte	0x48,0x83,0xEC,0x08
.cfi_adjust_cfa_offset	8
	movq	$-1,%rax
	testq	%rdi,%rdi
	jz	.Lenc_key_ret
	testq	%rdx,%rdx
	jz	.Lenc_key_ret

	movups	(%rdi),%xmm0
	xorps	%xmm4,%xmm4



	leaq	16(%rdx),%rax
	cmpl	$256,%esi
	je	.L14rounds
	cmpl	$192,%esi
	je	.L12rounds
	cmpl	$128,%esi
	jne	.Lbad_keybits

.L10rounds:
	movl	$9,%esi































	movdqa	.Lkey_rotate(%rip),%xmm5
	movl	$8,%r10d
	movdqa	.Lkey_rcon1(%rip),%xmm4
	movdqa	%xmm0,%xmm2
	movdqu	%xmm0,(%rdx)
	jmp	.Loop_key128

.align	16
.Loop_key128:
	pshufb	%xmm5,%xmm0
.byte	102,15,56,221,196
	pslld	$1,%xmm4
	leaq	16(%rax),%rax

	movdqa	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm3,%xmm2

	pxor	%xmm2,%xmm0
	movdqu	%xmm0,-16(%rax)
	movdqa	%xmm0,%xmm2

	decl	%r10d
	jnz	.Loop_key128

	movdqa	.Lkey_rcon1b(%rip),%xmm4

	pshufb	%xmm5,%xmm0
.byte	102,15,56,221,196
	pslld	$1,%xmm4

	movdqa	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm3,%xmm2

	pxor	%xmm2,%xmm0
	movdqu	%xmm0,(%rax)

	movdqa	%xmm0,%xmm2
	pshufb	%xmm5,%xmm0
.byte	102,15,56,221,196

	movdqa	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm2,%xmm3
	pslldq	$4,%xmm2
	pxor	%xmm3,%xmm2

	pxor	%xmm2,%xmm0
	movdqu	%xmm0,16(%rax)

	movl	%esi,96(%rax)
	xorl	%eax,%eax
	jmp	.Lenc_key_ret

.align	16
.L12rounds:
	movq	16(%rdi),%xmm2
	movl	$11,%esi



























	movdqa	.Lkey_rotate192(%rip),%xmm5
	movdqa	.Lkey_rcon1(%rip),%xmm4
	movl	$8,%r10d
	movdqu	%xmm0,(%rdx)
	jmp	.Loop_key192

.align	16
.Loop_key192:
	movq	%xmm2,0(%rax)
	movdqa	%xmm2,%xmm1
	pshufb	%xmm5,%xmm2
.byte	102,15,56,221,212
	pslld	$1,%xmm4
	leaq	24(%rax),%rax

	movdqa	%xmm0,%xmm3
	pslldq	$4,%xmm0
	pxor	%xmm0,%xmm3
	pslldq	$4,%xmm0
	pxor	%xmm0,%xmm3
	pslldq	$4,%xmm0
	pxor	%xmm3,%xmm0

	pshufd	$0xff,%xmm0,%xmm3
	pxor	%xmm1,%xmm3
	pslldq	$4,%xmm1
	pxor	%xmm1,%xmm3

	pxor	%xmm2,%xmm0
	pxor	%xmm3,%xmm2
	movdqu	%xmm0,-16(%rax)

	decl	%r10d
	jnz	.Loop_key192

	movl	%esi,32(%rax)
	xorl	%eax,%eax
	jmp	.Lenc_key_ret

.align	16
.L14rounds:
	movups	16(%rdi),%xmm2
	movl	$13,%esi
	leaq	16(%rax),%rax






































	movdqa	.Lkey_rotate(%rip),%xmm5
	movdqa	.Lkey_rcon1(%rip),%xmm4
	movl	$7,%r10d
	movdqu	%xmm0,0(%rdx)
	movdqa	%xmm2,%xmm1
	movdqu	%xmm2,16(%rdx)
	jmp	.Loop_key256

.align	16
.Loop_key256:
	pshufb	%xmm5,%xmm2
.byte	102,15,56,221,212

	movdqa	%xmm0,%xmm3
	pslldq	$4,%xmm0
	pxor	%xmm0,%xmm3
	pslldq	$4,%xmm0
	pxor	%xmm0,%xmm3
	pslldq	$4,%xmm0
	pxor	%xmm3,%xmm0
	pslld	$1,%xmm4

	pxor	%xmm2,%xmm0
	movdqu	%xmm0,(%rax)

	decl	%r10d
	jz	.Ldone_key256

	pshufd	$0xff,%xmm0,%xmm2
	pxor	%xmm3,%xmm3
.byte	102,15,56,221,211

	movdqa	%xmm1,%xmm3
	pslldq	$4,%xmm1
	pxor	%xmm1,%xmm3
	pslldq	$4,%xmm1
	pxor	%xmm1,%xmm3
	pslldq	$4,%xmm1
	pxor	%xmm3,%xmm1

	pxor	%xmm1,%xmm2
	movdqu	%xmm2,16(%rax)
	leaq	32(%rax),%rax
	movdqa	%xmm2,%xmm1

	jmp	.Loop_key256

.Ldone_key256:
	movl	%esi,16(%rax)
	xorl	%eax,%eax
	jmp	.Lenc_key_ret

.align	16
.Lbad_keybits:
	movq	$-2,%rax
.Lenc_key_ret:
	pxor	%xmm0,%xmm0
	pxor	%xmm1,%xmm1
	pxor	%xmm2,%xmm2
	pxor	%xmm3,%xmm3
	pxor	%xmm4,%xmm4
	pxor	%xmm5,%xmm5
	addq	$8,%rsp
.cfi_adjust_cfa_offset	-8
	ret
.cfi_endproc	
.LSEH_end_set_encrypt_key:





































































.size	aesni_set_encrypt_key,.-aesni_set_encrypt_key
.size	__aesni_set_encrypt_key,.-__aesni_set_encrypt_key
.align	64
.Lbswap_mask:
.byte	15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
.Lincrement32:
.long	6,6,6,0
.Lincrement64:
.long	1,0,0,0
.Lxts_magic:
.long	0x87,0,1,0
.Lincrement1:
.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1
.Lkey_rotate:
.long	0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d,0x0c0f0e0d
.Lkey_rotate192:
.long	0x04070605,0x04070605,0x04070605,0x04070605
.Lkey_rcon1:
.long	1,1,1,1
.Lkey_rcon1b:
.long	0x1b,0x1b,0x1b,0x1b

.align	64
